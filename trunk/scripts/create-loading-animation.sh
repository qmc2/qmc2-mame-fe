#!/bin/sh
#
# Creates the rotating loading animation data/img/classic/loadanim.gif from the single base
# image data/img/classic/animbase.png.
#
# Requires ImageMagick's convert tool!
#
command="convert -delay 10 data/img/classic/animbase.png"
command="$command -gravity center -background transparent -compose src"
for i in $(seq 5 5 360); do
	command="$command \\( -clone 0 -rotate ${i} -clone 0 +swap -composite \\)"
done
command="$command data/tmp/animation.png"
eval $command
frames=$(for i in $(seq 0 72); do echo data/tmp/animation-${i}.png; done)
convert -loop 0 -dispose 3 -delay 5 ${frames} -delete 0 -layers trim-bounds data/img/classic/loadanim.gif
rm -f ${frames}
