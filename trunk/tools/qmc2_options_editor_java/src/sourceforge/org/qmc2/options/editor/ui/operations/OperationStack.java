package sourceforge.org.qmc2.options.editor.ui.operations;

import java.util.Stack;

import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public final class OperationStack {

	private final Stack<IOperation> undoStack = new Stack<IOperation>();

	private final Stack<IOperation> redoStack = new Stack<IOperation>();

	public IStatus undo() {
		IStatus operationStatus = null;
		if (!undoStack.isEmpty()) {
			IOperation latestOp = undoStack.pop();
			try {
				operationStatus = latestOp.undo();
				redoStack.push(latestOp);
			} catch (Exception e) {
				operationStatus = new Status(IStatus.ERROR,
						QMC2Editor.EDITOR_ID, 0,
						"An error occurred executing undoable operation", e);
			}

		}

		return operationStatus;
	}

	public IStatus redo() {
		IStatus operationStatus = null;
		if (!redoStack.isEmpty()) {
			IOperation latestOp = redoStack.pop();
			try {
				operationStatus = latestOp.redo();
				undoStack.push(latestOp);
			} catch (Exception e) {
				operationStatus = new Status(IStatus.ERROR,
						QMC2Editor.EDITOR_ID, 0,
						"An error occurred executing undoable operation", e);
			}

		}

		return operationStatus;

	}

	/**
	 * Execute the desired operation and add it to the operations stack. If any
	 * error occur, the operation will not be added
	 * 
	 * @param operation
	 *            the operation to be executed
	 * @return the status of the operation execution
	 */
	public IStatus execute(IOperation operation) {
		IStatus operationStatus = null;

		try {
			operationStatus = operation.execute();
			undoStack.push(operation);
			redoStack.removeAllElements();
		} catch (Exception e) {
			operationStatus = new Status(IStatus.ERROR, QMC2Editor.EDITOR_ID,
					0, "An error occurred executing undoable operation", e);
		}

		return operationStatus;
	}

	public void clear() {
		undoStack.removeAllElements();
		redoStack.removeAllElements();
	}

	public boolean hasUndoOperations() {
		return undoStack.size() > 0;
	}

	public boolean hasRedoOperations() {
		return redoStack.size() > 0;
	}

}
