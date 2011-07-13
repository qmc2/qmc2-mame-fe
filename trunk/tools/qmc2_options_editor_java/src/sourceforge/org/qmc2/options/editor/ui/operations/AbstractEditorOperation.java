package sourceforge.org.qmc2.options.editor.ui.operations;

import org.eclipse.core.runtime.IStatus;
import org.eclipse.jface.viewers.ColumnViewer;

public abstract class AbstractEditorOperation implements IOperation {

	private final ColumnViewer viewer;

	public AbstractEditorOperation(ColumnViewer viewer) {
		this.viewer = viewer;
	}

	public abstract IStatus execute();

	public abstract IStatus redo();

	public abstract IStatus undo();

	public ColumnViewer getViewer() {
		return viewer;
	}

}
