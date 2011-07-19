package sourceforge.org.qmc2.options.editor.ui.operations;

import org.eclipse.core.commands.operations.AbstractOperation;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public abstract class AbstractEditorOperation extends AbstractOperation {

	private final QMC2Editor editor;

	public AbstractEditorOperation(QMC2Editor editor, String label) {
		super(label);
		this.editor = editor;
		addContext(editor.getUndoContext());
	}

	protected QMC2Editor getEditor() {
		return editor;
	}

}
