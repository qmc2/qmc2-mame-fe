package sourceforge.org.qmc2.options.editor.ui.operations;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;

import sourceforge.org.qmc2.options.editor.model.DescriptableItem;
import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class EditOperation extends AbstractEditorOperation {

	private final DescriptableItem item;

	private final String lang;

	private final String newValue;

	private final String oldValue;

	public EditOperation(QMC2Editor editor, DescriptableItem item, String lang,
			String value) {
		super(editor, "Edit");
		this.item = item;
		this.lang = lang;
		this.newValue = value;
		this.oldValue = item.getDescription(lang);
	}

	@Override
	public IStatus execute(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {
		return redo(monitor, info);
	}

	@Override
	public IStatus redo(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {
		IStatus operationStatus = Status.OK_STATUS;

		item.setDescription(lang, newValue);
		getEditor().getViewer().update(item, null);

		return operationStatus;
	}

	@Override
	public IStatus undo(IProgressMonitor monitor, IAdaptable info) {
		IStatus operationStatus = Status.OK_STATUS;

		item.setDescription(lang, oldValue);
		getEditor().getViewer().update(item, null);

		return operationStatus;
	}

}
