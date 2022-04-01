package sourceforge.org.qmc2.options.editor.ui.actions;

import java.util.Locale;

import org.eclipse.core.commands.operations.IUndoableOperation;
import org.eclipse.jface.dialogs.IInputValidator;
import org.eclipse.jface.dialogs.InputDialog;
import org.eclipse.jface.window.Window;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.TreeColumn;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;
import sourceforge.org.qmc2.options.editor.ui.operations.AddLanguageOperation;

public class AddLanguageAction extends BaseAction {

	public AddLanguageAction(QMC2Editor editor) {
		super(editor);
		setAccelerator(SWT.MOD1 + 'A');
		setText("&Add Language...");
	}

	@Override
	public boolean isEnabled() {
		return editor.getTemplateFile() != null;
	}

	@Override
	public void run() {
		InputDialog dialog = new InputDialog(editor.getShell(), "Add language",
				"Add the new language id", "", new IInputValidator() {

					@Override
					public String isValid(String newText) {
						String errorMsg = null;

						if (newText == null || newText.trim().length() <= 1) {
							errorMsg = "You must enter a language code from ISO 639";
						}

						if (errorMsg == null) {
							TreeColumn[] columns = editor.getViewer().getTree()
									.getColumns();

							for (int i = 0; i < columns.length
									&& errorMsg == null; i++) {
								if (columns[i].getText().equals(
										newText.toLowerCase())) {
									errorMsg = "Language already exists";
								}
							}
						}

						if (errorMsg == null) {
							String[] languages = Locale.getISOLanguages();
							boolean hasLanguage = false;
							for (int i = 0; i < languages.length
									&& errorMsg == null; i++) {
								if (languages[i].toLowerCase().equals(
										newText.toLowerCase())) {
									hasLanguage = true;
								}
							}
							if (!hasLanguage) {
								errorMsg = "You must enter a language code from ISO 639";
							}
						}

						return errorMsg;
					}
				});
		if (dialog.open() == Window.OK) {
			String newLang = dialog.getValue();
			IUndoableOperation operation = new AddLanguageOperation(editor,
					newLang);
			editor.executeOperation(operation);
		}
		super.run();
	}
}
