package sourceforge.org.qmc2.options.editor.ui;

import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.Viewer;

import sourceforge.org.qmc2.options.editor.model.DescriptableItem;
import sourceforge.org.qmc2.options.editor.model.QMC2TemplateFile;
import sourceforge.org.qmc2.options.editor.model.Section;

public class QMC2ContentProvider implements ITreeContentProvider {

	@Override
	public void dispose() {
		// do nothing

	}

	@Override
	public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {

	}

	@Override
	public Object[] getElements(Object inputElement) {

		if (inputElement instanceof QMC2TemplateFile) {
			return ((QMC2TemplateFile) inputElement).getSections().toArray();
		} else if (inputElement instanceof Section) {
			return ((Section) inputElement).getOptions().toArray();
		}

		return null;

	}

	@Override
	public Object[] getChildren(Object parentElement) {
		return getElements(parentElement);
	}

	@Override
	public Object getParent(Object element) {
		if (element instanceof DescriptableItem) {
			return ((DescriptableItem) element).getParent();
		}
		return null;
	}

	@Override
	public boolean hasChildren(Object element) {
		if (element instanceof QMC2TemplateFile) {
			return ((QMC2TemplateFile) element).getSections().size() > 0;
		} else if (element instanceof Section) {
			return ((Section) element).getOptions().size() > 0;
		}
		return false;
	}

}
