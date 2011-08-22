package sourceforge.org.qmc2.options.editor.ui.actions;

import org.eclipse.swt.SWT;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;
import sourceforge.org.qmc2.options.editor.ui.operations.RemoveItemsOperation;

public class RemoveSelectedItemsAction extends BaseAction {

	public RemoveSelectedItemsAction(QMC2Editor editor) {
		super(editor);
		setText("&Remove Items");
		setAccelerator(SWT.DEL);
	}

	@Override
	public void run() {
		RemoveItemsOperation operation = new RemoveItemsOperation(editor);
		editor.executeOperation(operation);
		super.run();
	}

	@Override
	public boolean isEnabled() {
		return editor.getTemplateFile() != null
				&& !editor.getViewer().getSelection().isEmpty();
	}
}
