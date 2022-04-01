package sourceforge.org.qmc2.options.editor.ui.actions;

import org.eclipse.jface.viewers.ITreeSelection;
import org.eclipse.jface.window.Window;

import sourceforge.org.qmc2.options.editor.model.Option;
import sourceforge.org.qmc2.options.editor.model.Section;
import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;
import sourceforge.org.qmc2.options.editor.ui.dialogs.AddOptionDialog;
import sourceforge.org.qmc2.options.editor.ui.operations.AddOptionOperation;

public class AddOptionAction extends BaseAction {

	public AddOptionAction(QMC2Editor editor) {
		super(editor);
		setText("Add Option...");
	}

	@Override
	public void run() {
		AddOptionDialog addOptionDialog = new AddOptionDialog(
				editor.getShell(), null);
		if (addOptionDialog.open() == Window.OK) {
			Section selectedSection = getCurrentSection();
			AddOptionOperation operation = new AddOptionOperation(editor,
					selectedSection, addOptionDialog.getOption());
			editor.executeOperation(operation);
		}
		super.run();
	}

	private Section getCurrentSection() {
		Section selectedSection = null;
		ITreeSelection selection = (ITreeSelection) editor.getViewer()
				.getSelection();
		Object element = selection.getFirstElement();
		if (element != null) {
			selectedSection = (element instanceof Section) ? ((Section) element)
					: (Section) ((Option) element).getParent();
		}
		return selectedSection;
	}

	@Override
	public boolean isEnabled() {
		return editor.getTemplateFile() != null;
	}
}
