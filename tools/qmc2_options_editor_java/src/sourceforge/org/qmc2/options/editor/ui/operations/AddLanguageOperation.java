package sourceforge.org.qmc2.options.editor.ui.operations;

import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.viewers.TreeViewerColumn;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class AddLanguageOperation extends AbstractEditorOperation {

	private final String lang;

	private TreeViewerColumn column;

	public AddLanguageOperation(QMC2Editor editor, String newLang) {
		super(editor);
		this.lang = newLang;

	}

	@Override
	public IStatus execute() {
		return redo();
	}

	@Override
	public IStatus redo() {
		column = getEditor().createColumn(getEditor().getViewer(), lang);
		column.getColumn().setWidth(100);
		getEditor().getViewer().refresh();
		return Status.OK_STATUS;
	}

	@Override
	public IStatus undo() {
		column.getColumn().dispose();
		getEditor().getViewer().refresh();
		return Status.OK_STATUS;
	}

}
