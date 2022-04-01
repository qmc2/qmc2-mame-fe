package sourceforge.org.qmc2.options.editor.ui.actions;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.operations.IUndoableOperation;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.swt.SWT;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class UndoAction extends BaseAction {

	public UndoAction(QMC2Editor editor) {
		super(editor);
		setAccelerator(SWT.MOD1 + 'Z');
	}

	@Override
	public boolean isEnabled() {
		return editor.getOperationHistory() != null
				&& editor.getOperationHistory()
						.canUndo(editor.getUndoContext());
	}

	@Override
	public void run() {
		try {
			editor.getOperationHistory().undo(editor.getUndoContext(),
					new NullProgressMonitor(), null);
		} catch (ExecutionException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		super.run();
	}

	@Override
	public String getText() {
		IUndoableOperation nextUndoOp = editor.getOperationHistory()
				.getUndoOperation(editor.getUndoContext());

		return "&Undo"
				+ (nextUndoOp == null ? "" : (" " + nextUndoOp.getLabel()));

	}

}
