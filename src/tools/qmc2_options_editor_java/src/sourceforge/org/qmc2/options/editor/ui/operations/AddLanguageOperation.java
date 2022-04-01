package sourceforge.org.qmc2.options.editor.ui.operations;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.viewers.TreeViewerColumn;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class AddLanguageOperation extends AbstractEditorOperation {

	private final String lang;

	private TreeViewerColumn column;

	public AddLanguageOperation(QMC2Editor editor, String newLang) {
		super(editor, "Add Language");
		this.lang = newLang;

	}

	@Override
	public IStatus execute(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {

		return redo(monitor, info);
	}

	@Override
	public IStatus redo(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {
		column = getEditor().createColumn(getEditor().getViewer(), lang, -1);
		column.getColumn().setWidth(100);
		getEditor().getViewer().refresh();
		return Status.OK_STATUS;
	}

	@Override
	public IStatus undo(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {
		column.getColumn().dispose();
		getEditor().getViewer().refresh();
		return Status.OK_STATUS;
	}

}
