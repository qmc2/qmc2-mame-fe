package sourceforge.org.qmc2.options.editor.ui;

import java.io.File;
import java.io.IOException;
import java.util.Set;

import org.eclipse.jface.dialogs.ErrorDialog;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.jface.viewers.TreeViewerColumn;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.TreeColumn;

import sourceforge.org.qmc2.options.editor.model.QMC2TemplateFile;

public class QMC2Editor extends Composite {

	private final TreeViewer viewer;

	private String selectedFile = null;

	private QMC2TemplateFile templateFile = null;

	public QMC2Editor(Composite parent) {
		super(parent, SWT.NONE);
		setLayout(new GridLayout(3, false));

		createFileChooser();

		viewer = new TreeViewer(this, SWT.SINGLE | SWT.V_SCROLL | SWT.H_SCROLL
				| SWT.BORDER);
		viewer.setContentProvider(new QMC2ContentProvider());
		viewer.getTree().setHeaderVisible(true);
		viewer.getTree().setLinesVisible(true);
		viewer.getTree().setLayoutData(
				new GridData(SWT.FILL, SWT.FILL, true, true, 3, 1));

		createSaveArea();

	}

	private void createSaveArea() {
		GridData layoutData = new GridData(SWT.TRAIL, SWT.TOP, false, false, 3,
				1);
		Button saveButton = new Button(this, SWT.PUSH);
		saveButton.setText("Save...");
		saveButton.setLayoutData(layoutData);
		saveButton.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				FileDialog dialog = new FileDialog(getShell(), SWT.SAVE);
				dialog.setFileName(selectedFile);
				String filename = dialog.open();
				if (filename != null) {
					File f = new File(filename);
					if (!f.exists()) {
						try {
							if (!f.createNewFile()) {
								MessageDialog
										.openError(getShell(), "Error",
												"Cannot save to selected file. Check your permissions");
							}
						} catch (IOException e1) {
							MessageDialog.openError(getShell(), "Error",
									"An exception ocurred creating file for saving: "
											+ e1.getMessage());
						}
					}

					if (f.canWrite() && f.isFile()) {

						try {
							templateFile.save(f);
						} catch (Exception e1) {
							MessageDialog.openError(getShell(), "Error",
									"An exception ocurred saving template file: "
											+ e1.getMessage());
						}
					} else {
						MessageDialog
								.openError(getShell(), "Error",
										"Cannot save to selected file. Check your permissions");
					}

				}
			}
		});

	}

	private void createFileChooser() {

		GridData layoutData = new GridData(SWT.LEAD, SWT.TOP, false, false, 1,
				1);
		Label fileSelectionLabel = new Label(this, SWT.NONE);
		fileSelectionLabel.setText("Template File: ");
		fileSelectionLabel.setLayoutData(layoutData);

		final Text fileSelection = new Text(this, SWT.BORDER | SWT.SINGLE);
		layoutData = new GridData(SWT.FILL, SWT.TOP, true, false, 1, 1);
		fileSelection.setLayoutData(layoutData);
		fileSelection.addModifyListener(new ModifyListener() {

			@Override
			public void modifyText(ModifyEvent e) {
				selectedFile = fileSelection.getText();
				File f = new File(selectedFile);
				if (f.exists() && f.isFile() && f.canRead()) {
					try {
						templateFile = QMC2TemplateFile.parse(new File(
								selectedFile));
						createColumns(templateFile.getLanguages());
						viewer.setInput(templateFile);
					} catch (Exception e1) {
						ErrorDialog.openError(getShell(), "Error",
								"Unable to open template file", null);
					}
				}

			}
		});

		Button browse = new Button(this, SWT.PUSH);
		browse.setText("Browse...");
		layoutData = new GridData(SWT.TRAIL, SWT.TOP, false, false, 1, 1);
		browse.setLayoutData(layoutData);

		browse.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				FileDialog dialog = new FileDialog(getShell(), SWT.OPEN);
				dialog.setFileName(selectedFile);
				String filename = dialog.open();
				if (filename != null) {
					selectedFile = filename;
					fileSelection.setText(filename);
				}
			}
		});

	}

	private void createColumns(Set<String> languages) {

		int KEYS_COLUMN_SIZE = 150;

		viewer.getTree().setRedraw(false);
		for (TreeColumn c : viewer.getTree().getColumns()) {
			c.dispose();
		}

		TreeViewerColumn c = new TreeViewerColumn(viewer, SWT.NONE);

		c.getColumn().setMoveable(false);
		c.getColumn().setText("Item");
		c.setLabelProvider(new QMC2LabelProvider(null));
		c.getColumn().setWidth(KEYS_COLUMN_SIZE);

		for (String lang : languages) {
			c = new TreeViewerColumn(viewer, SWT.NONE);
			c.getColumn().setMoveable(false);
			c.getColumn().setText(lang);
			c.setLabelProvider(new QMC2LabelProvider(lang));
		}

		viewer.getTree().setRedraw(true);
		int columnSize = (viewer.getTree().getSize().x - KEYS_COLUMN_SIZE)
				/ (viewer.getTree().getColumnCount() - 1);

		for (int i = 1; i < viewer.getTree().getColumnCount(); i++) {
			viewer.getTree().getColumn(i).setWidth(columnSize);
		}

	}

}
