package sourceforge.org.qmc2.options.editor.ui;

import org.eclipse.jface.viewers.ColumnLabelProvider;

import sourceforge.org.qmc2.options.editor.model.DescriptableItem;

public class QMC2LabelProvider extends ColumnLabelProvider {

	private final String lang;

	public QMC2LabelProvider(String lang) {
		this.lang = lang;
	}

	@Override
	public String getText(Object element) {
		String value = "";

		if (element instanceof DescriptableItem) {
			DescriptableItem item = (DescriptableItem) element;
			if (lang == null) {
				value = item.getName();
			} else {
				value = item.getDescription(lang);
			}
		}

		return value;
	}

}
