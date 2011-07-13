package sourceforge.org.qmc2.options.editor.ui.operations;

import org.eclipse.core.runtime.IStatus;

public interface IOperation {

	public IStatus execute();

	public IStatus redo();

	public IStatus undo();

}
