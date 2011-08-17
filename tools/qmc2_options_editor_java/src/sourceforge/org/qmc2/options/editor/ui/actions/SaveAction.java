package sourceforge.org.qmc2.options.editor.ui.actions;

import java.io.File;
import java.io.IOException;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class SaveAction extends BaseAction {

	public SaveAction(QMC2Editor editor) {
		super(editor);
		setAccelerator(SWT.MOD1 + 'S');
		setText("&Save...");
	}

	@Override
	public boolean isEnabled() {
		boolean enabled = super.isEnabled();
		String currentFile = editor.getCurrentFile();
		if (currentFile == null || currentFile.trim().length() == 0
				|| editor.getTemplateFile() == null) {
			enabled = false;
		}
		return enabled;
	}

	@Override
	public void run() {
		FileDialog dialog = new FileDialog(editor.getShell(), SWT.SAVE);
		dialog.setFileName(editor.getCurrentFile());
		String filename = dialog.open();
		if (filename != null) {
			File f = new File(filename);
			if (!f.exists()) {
				try {
					if (!f.createNewFile()) {
						MessageDialog
								.openError(editor.getShell(), "Error",
										"Cannot save to selected file. Check your permissions");
					}
				} catch (IOException e1) {
					MessageDialog.openError(editor.getShell(), "Error",
							"An exception ocurred creating file for saving: "
									+ e1.getMessage());
				}
			}

			if (f.canWrite() && f.isFile()) {

				try {
					editor.getTemplateFile().save(f);
				} catch (Exception e1) {
					MessageDialog.openError(
							editor.getShell(),
							"Error",
							"An exception ocurred saving template file: "
									+ e1.getMessage());
				}
			} else {
				MessageDialog.openError(editor.getShell(), "Error",
						"Cannot save to selected file. Check your permissions");
			}

		}
		super.run();
	}
}
