package sourceforge.org.qmc2.options.editor.ui.operations;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;

import sourceforge.org.qmc2.options.editor.model.Option;
import sourceforge.org.qmc2.options.editor.model.Section;
import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class AddOptionOperation extends AbstractEditorOperation {

	private final Section section;

	private final Option option;

	public AddOptionOperation(QMC2Editor editor, Section s, Option o) {
		super(editor, "Add Option");
		this.section = s;
		this.option = o;

	}

	@Override
	public IStatus execute(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {
		return redo(monitor, info);
	}

	@Override
	public IStatus redo(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {
		section.addOption(option);
		getEditor().getViewer().refresh(section);
		return Status.OK_STATUS;
	}

	@Override
	public IStatus undo(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {
		section.removeOption(option.getName());
		getEditor().getViewer().refresh(section);
		return Status.OK_STATUS;
	}

}
