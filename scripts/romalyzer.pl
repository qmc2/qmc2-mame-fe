#!/usr/bin/perl
##
## ROMAlyzer.pl
## Copyright (C) 2007, Carsten Engel (carsten.engel@maxi-dsl.de)
##
## TODO:
##
## ZIP: move impl. (incl. delete if empty)
## ZIP: remove directories impl. -> pack all files to root dir (unless there are multiple files with the same name)...
## own move-to-trash-mode for CHDs (disks), because uncompressed in directory <gamename>
## but keep CHDs of normal "ROM files" (distinguish by "attribute")
## scan installed ROMs for unused files
## Q: where does MAME actually find used files (cloneof, romof, ...)?
##
use strict;
use Getopt::Long;
use Digest::CRC;
use Digest::SHA;
use File::Copy;
use Archive::Zip qw( :ERROR_CODES :CONSTANTS );
use XML::LibXML;

my $MAMEEXE = "/path/to/mame/executable";    ## option: -me=".."
my $ROMPATH = "/path/to/mame/roms";          ## option: -rp=".."
my $SAMPLEPATH = "/path/to/mame/samples";    ## option: -sp=".."
my $PREVIEWPATH = "/path/to/mame/previews";  ## option: -pp=".."
my $TRASHPATH = '';                          ## option: -tp=".."
my $DEEPSCAN = '';                           ## option: -ds
my $GETCHECKSUMS = '';                       ## option: -gc=".."
my $PREVIEWTRASH = "";
my $SAMPLETRASH = "";
my $ROMTRASH = "";

GetOptions ('mameexe|me=s' => \$MAMEEXE,
	    'rompath|rp=s' => \$ROMPATH,
	    'samplepath|sp=s' => \$SAMPLEPATH,
	    'previewpath|pp|imagepath|ip=s' => \$PREVIEWPATH,
	    'trashpath|tp|quarantinedir|qd=s' => \$TRASHPATH,
	    'getchecksums|gc|checksums|cs=s' => \$GETCHECKSUMS,
	    'deepscan|ds' => \$DEEPSCAN );

if ($TRASHPATH) {
    die "ERROR: Trash directory $TRASHPATH does not exist!\n" if (! -d "$TRASHPATH" );
    my $PREVIEWTRASH = "$TRASHPATH/screenshots";

    mkdir ("$PREVIEWTRASH") if (! -d "$PREVIEWTRASH");
    die "ERROR: Screenshot Trash directory $PREVIEWTRASH does not exist and could not be created!\n" if (! -d "$PREVIEWTRASH" );

    my $SAMPLETRASH = "$TRASHPATH/samples";
    mkdir ("$SAMPLETRASH") if (! -d "$SAMPLETRASH");
    die "ERROR: Sample Trash directory $SAMPLETRASH does not exist and could not be created!\n" if (! -d "$SAMPLETRASH" );

    my $ROMTRASH = "$TRASHPATH/roms";
    mkdir ("$ROMTRASH") if (! -d "$ROMTRASH");
    die "ERROR: ROM Trash directory $ROMTRASH does not exist and could not be created!\n" if (! -d "$ROMTRASH" );
}

my $zip = Archive::Zip->new();
my $zip_name = "";
my $zip_changed = 0;
my %zip_members;

my %PreviewFiles;
my %PreviewsFound;
my %SampleFiles;
my %ROMFiles;
my %ROMsFound;

sub openZIP {
    my ($fname) = @_;

    if ("$fname" ne "$zip_name" ) {
###        print "reading new ZIP: $fname\n";
	$zip = Archive::Zip->new();
	if ($zip->read("$fname") != AZ_OK) {
	    print "ERROR reading ZIP $fname\n";
	    $zip_name = "";
	    $zip_changed = 0;
	    %zip_members = {};
	    return 0;
	}
	$zip_name = $fname;
	$zip_changed = 0;
	foreach ($zip->memberNames()) {
	    $zip_members{"$_"}++;
	}
    }
    return -1;
}
    
sub getZipMemberNames {
    my ($fname) = @_;

    openZIP($fname) or return ();
    return $zip->memberNames();
}

sub getZipMemberData {
    my ($fname,$membername) = @_;

    openZIP($fname) and $zip_members{"$membername"} or return (-1, '', '');

    my $member = $zip->memberNamed("$membername");
    my $size = $member->uncompressedSize();
    my $crc = $member->crc32String();
    my $sha ='';
    if ($DEEPSCAN) {
        $member->desiredCompressionMethod( COMPRESSION_STORED );
	my $status = $member->rewindData();
	if ($status != AZ_OK) {
	    print "ERROR in ZIP $fname ! Could not read $membername !\n";
	    print "  $status\n";
	    return (-1, '', '');
	}
	my $shad = Digest::SHA->new("sha1");
	while ( ! $member->readIsDone() ) {
            my ($dataref, $status2) = $member->readChunk();
	    if ( ($status2 != AZ_OK) && ($status2 != AZ_STREAM_END) ) {
	        print "ERROR in ZIP $fname ! Could not read $membername !\n";
	        print "  $status2\n";
	        $member->endRead();
	        return (-1, '', '');
	    }
	    $shad->add($$dataref);
	}
	$member->endRead();
	$sha = $shad->hexdigest;
    }
    return ($size, $crc, $sha);
}

sub getROMFileChecksums {
    my ($filename) = @_;
    my @res = stat("$filename");
    my $size = $res[7];
    my $ctx = Digest::CRC->new(type=>"crc32");
    open (IN, "$filename") or die "ERROR opening file $filename !\n";
    $ctx->addfile(*IN);
    my $crcdigest = $ctx->hexdigest;
    my $shadigest = '';
    if ($DEEPSCAN) {
        my $sha = Digest::SHA->new("sha1");
        seek (IN,0,"SEEK_SET");
        $sha->addfile(*IN);
        $shadigest = $sha->hexdigest;
    }
    close (IN);
    return ($size,$crcdigest,$shadigest);
}

sub getDATAChecksums {
    my ($dataref) = @_;
    my $ctx = Digest::CRC->new(type=>"crc32");
    my $sha = Digest::SHA->new("sha1");
    $ctx->add($$dataref);
    $sha->add($$dataref);
    my $crcdigest = $ctx->hexdigest;
    my $shadigest = $sha->hexdigest;
    return ($crcdigest,$shadigest);
}

sub getCHDFileChecksums {
## Usage: my ($md5, $sha1) = getCHDFileChecksums("$filename");
    my ($filename) = @_;

    my $MD5="";
    my $SHA1="";
    my $sign;
    open (IN, "$filename") or die "ERROR opening file $filename !\n";
    read (IN,$sign,8);
    if ("$sign" eq "MComprHD") {
        my $hdrlen;
	my $vrsn;
        read (IN,$hdrlen,8);
 	($hdrlen,$vrsn) = unpack("H8H8",$hdrlen);
#	my $headerlen = hex($hdrlen);
	my $version = hex($vrsn);

	if ( ($version >= 2) && ($version <= 3) ) {
#          CHD Versions 1-3 have MD5 checksum [16 bytes] at offset 44.
	    my $mist;
	    read(IN,$mist,28);
	    my $md;
	    read(IN,$md,16);
	    $MD5 = unpack("H32",$md);
	    if ($version == 3) {
#          CHD Version 3 has SHA1 checksum [20 bytes] included
	        read(IN,$mist,20);
	        my $sh;
	        read(IN,$sh,20),
	        $SHA1 = unpack("H40",$sh);
	    }
	} else {
	    $MD5 = -1;
	    $SHA1 = "ERROR: $filename is not a valid CHD version (need 2-3, is $version)";
	    print "ERROR: $filename is not a valid CHD version (need 2-3, is $version)\n";
	}
    } else {
        $MD5 = -1;
	$SHA1 = "ERROR: $filename is not a valid CHD file!";
	print "ERROR: $filename is not a valid CHD file!\n";
    }
    close (IN);
    return ($MD5, $SHA1);
}

sub movetoTrash {
    my ($which, $filename, $zipfile, $gamename) = @_;

    my $targetDir = $TRASHPATH;
    my $targetZip = "";

    if (lc($which) =~ /preview/) {
        $targetDir = "$PREVIEWTRASH";
	if ($zipfile) {
#          TODO: if preview Screenshots are stored in a zip, keep this zip structure in Trash also
	    $targetZip = $zipfile;
	    $targetZip =~ s/^.*\///g;
	}
    } elsif (lc($which) =~ /sample/) {
        $targetDir = "$SAMPLETRASH";
	$targetZip = $gamename . ".zip";
    } elsif ( (lc($which) =~ /disk/) || (lc($which) =~ /chd/) ) {
        $targetDir = "$ROMTRASH/$gamename";
    } elsif (lc($which) =~ /rom/) {
        $targetDir = "$ROMTRASH";
	$targetZip = $gamename . ".zip";
    } else {
        print "ERROR: unknown Trash target $which for File $filename\n";
	return 0;
    }

    if (! -d "$targetDir" ) {
        if (! mkdir ("$targetDir")) {
	    print "ERROR: could not create Trash Dir $targetDir\n";
	    return 0;
	}
    }

    my $filen = "$filename";
    $filen =~ s/^.*\///g;
    while (-r "$targetDir/$filen" ) {
        $filen =~ s/\./_\./g;
    }
    if ( $zipfile ) {
#      TODO:
        print "###############################\n";
        print "ZIP-Move not implemented yet. $filename \-\> $targetDir/$filen\n";
	print "       target ZIP=$targetZip\n" if ($targetZip);
        print "###############################\n";
    } else {
        print "INFO: moving $filename to $targetDir/$filen\n";
	print "INFO:        target ZIP=$targetZip\n" if ($targetZip);
     ## move("$filename", "$targetDir/$filen") or print "ERROR: could not move $filename to $targetDir/$filen\n";
    }
}


my $PreviewCount = 0;

sub addPreviewFile {
    my ($name, $gamename, $used, $filename, $zipfile) = @_;
    if ( defined($PreviewFiles{$name}) ) {
#      duplicate Filenames should not occur!
        print "ERROR: duplicate Preview: $name for game $PreviewFiles{$name}->{Gamename}\n";
        print "ERROR:   and $name for game $gamename (IGNORED!)\n";
#      TODO:
##  move to trash? !!! (zipfile or not / other?)
    } else {
#      if we already found a Preview for that game, print a warning.
        if ( defined($PreviewsFound{$gamename}) ) {
	    print "WARNING: duplicate Previews for $gamename:\n";
	    print "WARNING:   $PreviewsFound{$gamename} and $name\n";
	    movetoTrash("preview", $filename, $zipfile, $gamename);
	} else {
	    $PreviewsFound{$gamename} = $name;
	    $PreviewCount++;
	}
        $PreviewFiles{$name} = { "Gamename" => $gamename, "Used" => $used, "Filename" => $filename, "Zipfile" => $zipfile };
    }
}

my $SampleCount = 0;

sub addSampleFile {
    my ($name, $gamename, $used, $filename, $zipfile) = @_;
    if ( defined($SampleFiles{$name}) ) {
#      duplicate Filenames should not occur!
        print "ERROR: duplicate Sample: $name for game $SampleFiles{$name}->{Gamename}\n";
	print "ERROR:   and $name for game $gamename (IGNORED!)\n";
#      TODO:
##  move to trash? !!!
    } else {
	$SampleCount++;

        $SampleFiles{$name} = { "Gamename" => $gamename, "Used" => $used, "Filename" => $filename, "Zipfile" => $zipfile };
    }
}

sub CheckSample {
    my ($SampleName, $GameName, $Mark) = @_;

    foreach my $fname (keys %SampleFiles) {
        if ( ( $SampleFiles{$fname}->{Gamename} eq "$GameName" ) && ( $SampleFiles{$fname}->{Filename} eq "$SampleName" ) ) {
	    $SampleFiles{$fname}->{Used} = 1 if ($Mark);
	    return -1;
	}
    }
    return 0;
}

my $ROMCount = 0;
my $CHDCount = 0;

sub addROMFile {
    my ($name, $gamename, $used, $filename, $zipfile, $type, $checksums) = @_;
    if ( defined($ROMFiles{"$gamename/$name"}) ) {
#      duplicate Filenames should not occur!
        print "ERROR: duplicate ROMFile $gamename/$name : " . $ROMFiles{"$gamename/$name"}->{Filename} . "\n";
	print "ERROR:   and $filename (IGNORED!)\n";
#      TODO:
##  move to trash? !!!
    } else {
        $ROMCount++;
        $CHDCount++ if ($type == 1);
        $ROMFiles{"$gamename/$name"} = { "Used" => $used, "Filename" => $filename, "Zipfile" => $zipfile, "Type" => $type, "Checksums" => $checksums };
	$ROMsFound{"$checksums"} = "$gamename/$name" if (!defined $ROMsFound{"$checksums"});
####
	print "INFO: ROMFile added $filename ($gamename/$name), Type=$type\n";
	print "INFO:    Checksums=$checksums\n";
####	print "INFO:    (ZIPPED in $zipfile)\n" if ($zipfile);
####
    }
}

#
# End of Soubroutine definition, start of main Program
#

# $GETCHECKSUMS indicates a single run to get the checksums for a directory
# or file

if ( $GETCHECKSUMS ) {
    if ( -d "$GETCHECKSUMS" ) {
####
#### TODO:
#### scan directory (NEEDS to be implemented!!!)...
####
    } elsif ( -f "$GETCHECKSUMS" ) {
#      always perform deep scan for single file Checksum
        $DEEPSCAN = 1;
        if ( "$GETCHECKSUMS" =~ /\.zip$/i ) {
#          ZIP file
            foreach my $zfname (getZipMemberNames("$GETCHECKSUMS")) {
#              skip directory entries in ZIP
	        next if ($zfname =~ /\/$/);
                my ($size, $crc, $sha) = getZipMemberData("$GETCHECKSUMS", "$zfname");
		print "Checksums for $GETCHECKSUMS/$zfname (Size: $size):\n      CRC:$crc  SHA1:$sha\n";
	    }
	} elsif ( "$GETCHECKSUMS" =~ /\.chd$/i ) {
#          CHD file
	    my ($md5, $sha1) = getCHDFileChecksums($GETCHECKSUMS);
	    print "Checksums for $GETCHECKSUMS:\n      SHA1:$sha1  MD5:$md5\n";
	} else {
#          plain file
            my ($size, $crc, $sha) = getROMFileChecksums("$GETCHECKSUMS");
	    print "Checksums for $GETCHECKSUMS (Size: $size):\n      CRC:$crc  SHA1:$sha\n";
	}
    } else {
        print "ERROR: improper value (no file or dir) for getchecksums: $GETCHECKSUMS\n";
    }

    exit;
}

my $PreviewSearch = 1;

if ($PreviewSearch) {
print "INFO: searching for installed Previews ...\n";
print "INFO: Preview-Path=$PREVIEWPATH\n";
my @previewdirs = split(/\:/, $PREVIEWPATH);
foreach my $dir (@previewdirs) {
    print "INFO: searching Dir $dir\n";
    opendir (DIR, $dir) or next;
    while (defined (my $file = readdir(DIR))){
	next if ($file =~ /^\.+$/ );
	if ( -d "$dir/$file"  ) {
	    push (@previewdirs, "$dir/$file");
##	    print "INFO: found SUBDIR: $dir/$file\n";
	    next;
	}
	my $filename = lc($file);
	my $gamename = $filename;
	$gamename =~ s/\..+$//;
	my $zipname = "";
	my %files;
	if ($filename =~ /\.zip$/i) {
	    $zipname = "$dir/$file";
            print "INFO: searching ZIP $zipname\n";
            foreach my $zfname (getZipMemberNames("$zipname")) {
                $gamename = $zfname;
		$gamename =~ s/^.*\///g;
		$gamename =~ s/\..+$//;

                $files{"$zipname/$zfname"} = $gamename;
            }
	} else {
	    $files{"$dir/$file"} = $gamename;
	}

        while (my ($fname, $gname) = each %files) {
	    if ( ($fname =~ /\.png$/i) || ($fname =~ /\.jpg$/i) || ($fname =~ /\.gif$/i) ) {
##	        print "INFO: found Preview $fname (game:$gname) ";
##	        print "(ZIPPED in $zipname)" if $zipname;
##	        print "\n";
	        my $name = "$fname";
	        $name =~ s/^.*\///g;
	        addPreviewFile("$fname", "$gname", 0, "$name", "$zipname");
	    } else {
	        if ( ($zipname) && ($fname =~ /\/$/) ) {
		    $fname =~ s/^$zipname\///g;
		    print "WARNING: unneeded directory structure in ZIP $zipname: $fname\n";
		} else {
	            print "WARNING: ignoring unknown file $fname ";
	            print "WARNING:   (ZIPPED in $zipname)" if $zipname;
		    print "\n";
		}
		next;
	    }
	}
    }
    closedir (DIR);
}

print "INFO: found $PreviewCount preview (screenshot) files\n";
}

my $SampleSearch = 1;

if ($SampleSearch) {
print "INFO: searching for installed Samples ...\n";
print "INFO: Sample-Path=$SAMPLEPATH\n";
my @sampledirs = split(/\:/, $SAMPLEPATH);
foreach my $dir (@sampledirs) {
    print "INFO: searching Dir $dir\n";
    opendir (DIR, $dir) or next;
    while (defined (my $file = readdir(DIR))){
        next if ($file =~ /^\.+$/ );
	if ( -d "$dir/$file"  ) {
	    push (@sampledirs, "$dir/$file");
##          print "INFO: found SUBDIR: $dir/$file\n";
            next;
	}
	my $filename = lc($file);
	my $gamename = $filename;
	$gamename =~ s/\..+$//;
	my $zipname = "";
	my %files;
	if ($filename =~ /\.zip$/i) {
	    $zipname = "$dir/$file";
	    print "INFO: searching ZIP $zipname\n";
	    foreach my $zfname (getZipMemberNames("$zipname")) {
	        $files{"$zipname/$zfname"} = $gamename;
	    }
	} else {
	    $gamename = $dir;
	    $gamename =~ s/^.*\///g;
	    $files{"$dir/$file"} = $gamename;
	}

	while ( my ($fname, $gname) = each %files) {
	    if ($fname =~ /\.wav$/i) {
##                print "INFO: found Sample $fname (game:$gname) ";
##                print "(ZIPPED in $zipname)" if $zipname;
##                print "\n";
                my $name = "$fname";
		$name =~ s/^.*\///g;
		addSampleFile("$fname", "$gname", 0, "$name", "$zipname");
            } else {
                if ( ($zipname) && ($fname =~ /\/$/) ) {
		    $fname =~ s/^$zipname\///g;
		    print "WARNING: unneeded directory structure in ZIP $zipname: $fname\n";
		} else {
		    print "WARNING: ignoring unknown file $fname ";
		    print "WARNING:   (ZIPPED in $zipname)" if $zipname;
		    print "\n";
		}
		next;
	    }
	}
    }
    closedir (DIR);
}

print "INFO: found $SampleCount Sample files\n";
}

my $RomSearch = 1;

if ($RomSearch) {
print "INFO: searching for installed ROMs ...\n";
print "INFO: performing deep ZIP scan (this may take a while).\n" if ($DEEPSCAN);
print "INFO: ROM-Path=$ROMPATH\n";
my @romdirs = split(/\:/, $ROMPATH);
foreach my $dir (@romdirs) {
    print "INFO: searching Dir $dir\n";
    opendir (DIR, $dir) or next;
    while (defined (my $file = readdir(DIR))){
        next if ($file =~ /^\.+$/ );
		if ( -d "$dir/$file"  ) {
			push (@romdirs, "$dir/$file");
##          print "INFO: found SUBDIR: $dir/$file\n";
            next;
		}
		my $filename = lc($file);
		my $gamename = $filename;
		$gamename =~ s/\..+$//;
		my $zipname = "";
		my %files;
		if ($filename =~ /\.zip$/i) {
			$zipname = "$dir/$file";
			print "INFO: searching ZIP $zipname\n";
			foreach my $zfname (getZipMemberNames("$zipname")) {
				$files{"$zipname/$zfname"} = $gamename;
			}
		} else {
			if (("$ROMPATH" =~ /$dir:/ ) || ("$ROMPATH" =~ /$dir$/)) {
				$gamename = "##UNKNOWN##";
			} else {
				$gamename = $dir;
				$gamename =~ s/^.*\///g;
			}
			$files{"$dir/$file"} = $gamename;
		}

		while ( my ($fname, $gname) = each %files) {
            my $name = "$fname";
			$name =~ s/^.*\///g;
			my $type = 0;
			my $checksums = "";
			if (lc($name) =~ /\.chd$/) {
				$type = 1;
##                print "INFO: found CHD File $fname (game:$gname) ";
##                print "(ZIPPED in $zipname)" if $zipname;
##                print "\n";
                if ($zipname) {
					print "ERROR: ZIPPED CHD not supported ($fname)\n";
					next;
#      TODO:
####             (evtl. automatisch auspacken).
				} else {
                    my ($md5, $sha1) = getCHDFileChecksums($fname);
					$checksums = "$md5|$sha1";
				}
			} elsif ( ($zipname) && ($fname =~ /\/$/) ) {
				my $tfname = "$fname";
				$tfname =~ s/^$zipname\///g;
				print "WARNING: unneeded directory structure in ZIP $zipname: $tfname\n";
				next;
			} else {
				if ($zipname) {
					my $tfname = "$fname";
					$tfname =~ s/^$zipname\///g;
                    my ($size,$crc,$sha) = getZipMemberData("$zipname", "$tfname");
					if (! $size) {
						print "ERROR: unable to extract ZIP-Data: $zipname -> $tfname\n";
						next;
					}
					$checksums = "$size|$crc|$sha";
                } else {
					my ($size, $crc, $sha) = getROMFileChecksums("$fname");
					$checksums = "$size|$crc|$sha";
				}
			}
			addROMFile("$name", "$gname", 0, "$fname", "$zipname", $type, "$checksums");
		}
    }
    closedir (DIR);
}

print "INFO: found $ROMCount ROM files (incl. $CHDCount CHDs)\n";
}

print "INFO: calling \"$MAMEEXE -lx\"\n";
my $parser = XML::LibXML->new();

my $handle;
open($handle,'-|',"$MAMEEXE -lx") or die "FATAL ERROR: cannot open MAME: $!\n";

my $doc = $parser->parse_fh( $handle );

close($handle);

print "INFO:  done... parsing output...\n";

foreach my $game ($doc->findnodes('/mame/game')) {
#  get the needed attributes
    my $name = $game->getAttribute("name");
    my $runnable = $game->getAttribute("runnable");
    my $cloneof = $game->getAttribute("cloneof");
    my $sample = $game->getAttribute("sampleof");
    my $romof = $game->getAttribute("romof");
    my $screenshot = "";
    my $driver = "";
    my $emulation = "";
    my $description = "";

#  get description text (Full Name)
    my @desc = $game->findnodes('./description');
    foreach (@desc) {
        $description .= " " if ($description);
	$description .= $_->to_literal;
    }

#  get needed Samples and check them
    my $SampleStatus = "";
    my @samples = $game->findnodes('./sample');
    if ( @samples ) {
        $SampleStatus = "INFO: Samples";
	$SampleStatus .= "\(of $sample\)" if ($sample);
	$SampleStatus .= ": ";
	my $sgname = $name;
	$sgname = $sample if ($sample);
	foreach (@samples) {
	    my $sname = $_->getAttribute("name");
	    $SampleStatus .= " $sname(";
	    if ( CheckSample($sname, $sgname, 1) ) {
	        $SampleStatus .= "OK";
	    } else {
	        $SampleStatus .= "ERROR: not found";
	    }
	    $SampleStatus .= ")";
	}
    }
#  print status summary for the game
    if ($SampleStatus) {
    print "Name: $name \(runnable:$runnable driver:$driver emul:$emulation\)";
    print " \"$description\"" if ($description);
    print ", clone of: $cloneof" if ($cloneof);
    print ", ROM of: $romof" if ($romof);
    print ", sample: $sample" if ($sample);
    print ", Screenshot: $screenshot" if ($screenshot);
    print "\n";
    print "$SampleStatus\n" if ($SampleStatus);
    }
    if ("$runnable" eq "yes") {
#      if the game is runnable, we check the driver status
        foreach my $drv ($game->findnodes('./driver')) {
	    $driver = $drv->getAttribute("status");
	    $emulation = $drv->getAttribute("emulation");
	}
#      and look for a screenshot
        $screenshot = "ERROR: NOT FOUND! ";
	if (defined $PreviewsFound{$name}) {
	    $screenshot = "$PreviewsFound{$name}";
	    $PreviewFiles{$screenshot}->{Used} = 1 if (defined $PreviewFiles{$screenshot});
	} elsif ( ($cloneof) && (defined $PreviewsFound{$cloneof}) ) {
	    $screenshot .= "(found Screenshot for parent: $PreviewsFound{$cloneof}) ";
	}

    }
    foreach my $rom ($game->findnodes('./rom')) {
        my $rname = $rom->getAttribute("name");
	my $rstatus = $rom->getAttribute("status");
	print "$name->$rname \($rstatus\)";
	if ( "$rstatus" ne "nodump" ) {
            my $rsize = $rom->getAttribute("size");
            my $rcrc = $rom->getAttribute("crc");
            my $rsha = $rom->getAttribute("sha1");
	    print ", size: $rsize, CRC: $rcrc, SHA1: $rsha";
	    my $rdispose = $rom->getAttribute("dispose");
	    $rdispose ="" if ("$rdispose" eq "no");
	    print ", dispose=$rdispose" if ($rdispose);
	    my $rmerge = $rom->getAttribute("merge");
	    print ", merge: $rmerge\($cloneof\)" if ($rmerge);
	    my $rmd5 = $rom->getAttribute("md5");
	    print ", MD5: $rmd5" if ($rmd5);
	}
	print "\n";
    }
    foreach my $disk ($game->findnodes('./disk')) {
        my $dname = $disk->getAttribute("name");
	my $dstatus = $disk->getAttribute("status");
	print "$name->$dname \($dstatus\)";
	if ( "$dstatus" ne "nodump" ) {
	    my $dsha  = $disk->getAttribute("sha1");
	    print ", SHA1: $dsha" if ($dsha);
	    my $dmd5 = $disk->getAttribute("md5");
	    print ", MD5: $dmd5" if ($dmd5);
	    my $dmerge = $disk->getAttribute("merge");
	    print ", merge: $dmerge\($cloneof\)" if ($dmerge);
	}
	print "\n";
    }
}

# scan installed Previews for unused files
foreach my $pfile (sort keys %PreviewFiles) {
    if ($PreviewFiles{"$pfile"}->{Used} != 1) {
        print "WARNING: unused Preview File $pfile";
	if ($PreviewFiles{$pfile}->{Zipfile}) {
	    print ", zipped in $PreviewFiles{$pfile}->{Zipfile}\n";
	} else {
	    print "\n";
        }
###	movetoTrash("preview", $pfile, $PreviewFiles{$pfile}->{Zipfile}, $PreviewFiles{$pfile}->{Gamename} );
    }
}

# scan installed Samples for unused files
foreach my $sfile (sort keys %SampleFiles) {
    if ($SampleFiles{"$sfile"}->{Used} != 1) {
        print "WARNING: unused Sample File $sfile";
	if ($SampleFiles{$sfile}->{Zipfile}) {
	    print ", zipped in $SampleFiles{$sfile}->{Zipfile}\n";
	} else {
	    print "\n";
        }
###	movetoTrash("sample", $sfile, $SampleFiles{$sfile}->{Zipfile}, $SampleFiles{$sfile}->{Gamename} );
    }
}
