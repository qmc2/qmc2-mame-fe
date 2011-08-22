package sourceforge.org.qmc2.options.editor.ui.actions;

import org.eclipse.core.commands.operations.IUndoableOperation;
import org.eclipse.jface.window.Window;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;
import sourceforge.org.qmc2.options.editor.ui.dialogs.AddSectionDialog;
import sourceforge.org.qmc2.options.editor.ui.operations.AddSectionOperation;

public class AddSectionAction extends BaseAction {

	public AddSectionAction(QMC2Editor editor) {
		super(editor);
		setText("Add &Section...");
	}

	@Override
	public boolean isEnabled() {
		return editor.getTemplateFile() != null;
	}

	@Override
	public void run() {
		AddSectionDialog dialog = new AddSectionDialog(editor.getShell(), null);
		if (dialog.open() == Window.OK) {
			IUndoableOperation operation = new AddSectionOperation(editor,
					dialog.getSection());
			editor.executeOperation(operation);
		}
		super.run();

	}

}
