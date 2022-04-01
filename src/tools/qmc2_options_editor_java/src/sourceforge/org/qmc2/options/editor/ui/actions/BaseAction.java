package sourceforge.org.qmc2.options.editor.ui.actions;

import org.eclipse.jface.action.Action;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class BaseAction extends Action {

	protected final QMC2Editor editor;

	public BaseAction(QMC2Editor editor) {
		this.editor = editor;
	}

	@Override
	public void run() {
		editor.updateApplicationMenuBar();
	}
}
