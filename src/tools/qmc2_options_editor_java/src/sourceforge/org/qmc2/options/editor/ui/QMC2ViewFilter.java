package sourceforge.org.qmc2.options.editor.ui;

import java.util.Iterator;
import java.util.Set;

import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerFilter;

import sourceforge.org.qmc2.options.editor.model.DescriptableItem;
import sourceforge.org.qmc2.options.editor.model.Option;
import sourceforge.org.qmc2.options.editor.model.Section;

public class QMC2ViewFilter extends ViewerFilter {

	private final QMC2Editor editor;

	public QMC2ViewFilter(QMC2Editor editor) {
		this.editor = editor;
	}

	@Override
	public boolean select(Viewer viewer, Object parentElement, Object element) {
		boolean canShow = false;
		if (editor.getFilter() != null) {
			if (element instanceof DescriptableItem) {
				DescriptableItem item = (DescriptableItem) element;
				canShow = canShow(item, editor.getFilter());
				/*
				 * Only hide sections if all children can be hidden
				 */
				if (item instanceof Section) {
					Iterator<Option> iterator = ((Section) item).getOptions()
							.iterator();
					while (iterator.hasNext() && !canShow) {
						canShow = canShow
								|| canShow(iterator.next(), editor.getFilter());
					}
				}
			}
		} else {
			canShow = true;
		}
		return canShow;
	}

	public boolean canShow(DescriptableItem item, String filterString) {
		boolean canShow = false;

		Set<String> languages = item.getLanguages();
		Iterator<String> iterator = languages.iterator();
		while (!canShow && iterator.hasNext()) {
			String lang = iterator.next();
			String description = item.getDescription(lang).toLowerCase();
			;
			if (description == null
					|| !description.contains(editor.getFilter())) {
				canShow = canShow || false;
			} else {
				canShow = canShow || true;
			}
		}
		if (item.getName().contains(editor.getFilter())) {
			canShow = canShow || true;
		} else {
			canShow = canShow || false;
		}

		return canShow;
	}

}
