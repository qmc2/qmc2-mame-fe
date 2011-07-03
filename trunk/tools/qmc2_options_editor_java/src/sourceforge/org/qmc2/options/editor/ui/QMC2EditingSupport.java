package sourceforge.org.qmc2.options.editor.ui;

import org.eclipse.jface.viewers.CellEditor;
import org.eclipse.jface.viewers.ColumnViewer;
import org.eclipse.jface.viewers.EditingSupport;
import org.eclipse.jface.viewers.TextCellEditor;
import org.eclipse.swt.widgets.Composite;

import sourceforge.org.qmc2.options.editor.model.DescriptableItem;

public class QMC2EditingSupport extends EditingSupport {

	private final String lang;

	private CellEditor editor;

	public QMC2EditingSupport(ColumnViewer viewer, String language) {
		super(viewer);
		this.lang = language;
		this.editor = new TextCellEditor((Composite) viewer.getControl());

	}

	@Override
	protected boolean canEdit(Object item) {
		return item instanceof DescriptableItem;
	}

	@Override
	protected CellEditor getCellEditor(Object item) {
		return editor;
	}

	@Override
	protected Object getValue(Object item) {
		String value = null;
		if (item instanceof DescriptableItem) {
			value = ((DescriptableItem) item).getDescription(lang);
		}

		return value;
	}

	@Override
	protected void setValue(Object item, Object value) {
		if (item instanceof DescriptableItem) {
			((DescriptableItem) item).setDescription(lang, value.toString());
			getViewer().refresh(item);
		}

	}

}
