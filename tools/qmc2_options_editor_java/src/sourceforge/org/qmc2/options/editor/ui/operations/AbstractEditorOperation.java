package sourceforge.org.qmc2.options.editor.ui.operations;

import org.eclipse.core.runtime.IStatus;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public abstract class AbstractEditorOperation implements IOperation {

	private final QMC2Editor editor;

	public AbstractEditorOperation(QMC2Editor editor) {
		this.editor = editor;
	}

	public abstract IStatus execute();

	public abstract IStatus redo();

	public abstract IStatus undo();

	protected QMC2Editor getEditor() {
		return editor;
	}

}
