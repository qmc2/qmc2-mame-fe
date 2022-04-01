package sourceforge.org.qmc2.options.editor.ui.operations;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.viewers.ITreeSelection;

import sourceforge.org.qmc2.options.editor.model.Option;
import sourceforge.org.qmc2.options.editor.model.Section;
import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class RemoveItemsOperation extends AbstractEditorOperation {

	private final List<Section> sectionsToRemove = new ArrayList<Section>();

	private final List<Option> optionsToRemove = new ArrayList<Option>();

	public RemoveItemsOperation(QMC2Editor editor) {
		super(editor, "Remove selected items");
	}

	@Override
	public IStatus execute(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {

		ITreeSelection selection = (ITreeSelection) getEditor().getViewer()
				.getSelection();
		for (Object o : selection.toList()) {
			if (o instanceof Option) {
				optionsToRemove.add((Option) o);
			} else if (o instanceof Section) {
				sectionsToRemove.add((Section) o);
			}
		}

		/**
		 * Remove options from options list if its parent (section) is marked to
		 * be removed
		 */
		for (Section s : sectionsToRemove) {
			for (Option o : s.getOptions()) {
				optionsToRemove.remove(o);
			}
		}

		return redo(monitor, info);
	}

	@Override
	public IStatus redo(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {
		for (Section s : sectionsToRemove) {
			getEditor().getTemplateFile().removeSection(s.getName());
		}
		for (Option o : optionsToRemove) {
			((Section) o.getParent()).removeOption(o.getName());
		}
		getEditor().getViewer().refresh();
		return Status.OK_STATUS;
	}

	@Override
	public IStatus undo(IProgressMonitor monitor, IAdaptable info)
			throws ExecutionException {

		List<Option> optionsToAdd = new ArrayList<Option>(optionsToRemove);
		Collections.reverse(optionsToAdd);

		List<Section> sectionsToAdd = new ArrayList<Section>(sectionsToRemove);
		Collections.reverse(sectionsToAdd);

		for (Option o : optionsToAdd) {
			((Section) o.getParent()).addOption(o, o.getIndex());
		}
		for (Section s : sectionsToAdd) {
			getEditor().getTemplateFile().addSection(s, s.getIndex());
		}
		getEditor().getViewer().refresh();
		return Status.OK_STATUS;
	}

}
