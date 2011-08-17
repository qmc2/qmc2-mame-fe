package sourceforge.org.qmc2.options.editor.ui.actions;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.operations.IUndoableOperation;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.swt.SWT;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class RedoAction extends BaseAction {

	public RedoAction(QMC2Editor editor) {
		super(editor);
		setAccelerator(SWT.MOD1 + 'Y');
	}

	@Override
	public boolean isEnabled() {
		return editor.getOperationHistory() != null
				&& editor.getOperationHistory()
						.canRedo(editor.getUndoContext());
	}

	@Override
	public void run() {
		try {
			editor.getOperationHistory().redo(editor.getUndoContext(),
					new NullProgressMonitor(), null);
		} catch (ExecutionException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		super.run();
	}

	@Override
	public String getText() {
		IUndoableOperation nextRedoOp = editor.getOperationHistory()
				.getRedoOperation(editor.getUndoContext());

		return "&Redo"
				+ (nextRedoOp == null ? "" : (" " + nextRedoOp.getLabel()));
	}
}
