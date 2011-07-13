package sourceforge.org.qmc2.options.editor.ui.operations;

import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.viewers.ColumnViewer;

import sourceforge.org.qmc2.options.editor.model.DescriptableItem;

public class EditOperation extends AbstractEditorOperation {

	private final DescriptableItem item;

	private final String lang;

	private final String newValue;

	private final String oldValue;

	public EditOperation(ColumnViewer viewer, DescriptableItem item,
			String lang, String value) {
		super(viewer);
		this.item = item;
		this.lang = lang;
		this.newValue = value;
		this.oldValue = item.getDescription(lang);
	}

	@Override
	public IStatus execute() {
		return redo();
	}

	@Override
	public IStatus redo() {
		IStatus operationStatus = Status.OK_STATUS;

		item.setDescription(lang, newValue);
		getViewer().update(item, null);

		return operationStatus;
	}

	@Override
	public IStatus undo() {
		IStatus operationStatus = Status.OK_STATUS;

		item.setDescription(lang, oldValue);
		getViewer().update(item, null);

		return operationStatus;
	}

}
