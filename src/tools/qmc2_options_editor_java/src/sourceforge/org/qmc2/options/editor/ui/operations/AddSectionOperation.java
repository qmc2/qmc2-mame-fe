package sourceforge.org.qmc2.options.editor.ui.operations;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;

import sourceforge.org.qmc2.options.editor.model.Section;
import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class AddSectionOperation extends AbstractEditorOperation {

	private final Section section;

	public AddSectionOperation(QMC2Editor editor, Section section) {
		super(editor, "Add Section");
		this.section = section;
	}

	@Override
	public IStatus execute(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {
		return redo(monitor, info);
	}

	@Override
	public IStatus redo(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {
		getEditor().getTemplateFile().addSection(section);
		getEditor().getViewer().refresh();
		return Status.OK_STATUS;
	}

	@Override
	public IStatus undo(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {
		getEditor().getTemplateFile().removeSection(section.getName());
		getEditor().getViewer().refresh();
		return Status.OK_STATUS;
	}

}
