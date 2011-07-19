package sourceforge.org.qmc2.options.editor.ui.actions;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.jface.action.Action;
import org.eclipse.swt.SWT;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class RedoAction extends Action {
	private final QMC2Editor editor;

	public RedoAction(QMC2Editor editor) {
		this.editor = editor;
		setAccelerator(SWT.MOD1 + 'Y');
		setText("&Redo");
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
	}
}
