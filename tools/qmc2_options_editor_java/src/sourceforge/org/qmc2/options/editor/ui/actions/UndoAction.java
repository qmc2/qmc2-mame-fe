package sourceforge.org.qmc2.options.editor.ui.actions;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.jface.action.Action;
import org.eclipse.swt.SWT;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class UndoAction extends Action {

	private final QMC2Editor editor;

	public UndoAction(QMC2Editor editor) {
		this.editor = editor;
		setAccelerator(SWT.MOD1 + 'Z');
		setText("&Undo");
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
	}

}
