package sourceforge.org.qmc2.options.editor.ui;

import org.eclipse.jface.viewers.CellEditor;
import org.eclipse.jface.viewers.EditingSupport;
import org.eclipse.jface.viewers.TextCellEditor;
import org.eclipse.swt.widgets.Composite;

import sourceforge.org.qmc2.options.editor.model.DescriptableItem;
import sourceforge.org.qmc2.options.editor.ui.operations.EditOperation;
import sourceforge.org.qmc2.options.editor.ui.operations.IOperation;

public class QMC2EditingSupport extends EditingSupport {

	private final String lang;

	private final CellEditor editor;

	private final QMC2Editor qmc2Editor;

	public QMC2EditingSupport(QMC2Editor editor, String language) {
		super(editor.getViewer());
		this.lang = language;
		this.qmc2Editor = editor;
		this.editor = new TextCellEditor((Composite) editor.getViewer()
				.getControl());

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
			IOperation operation = new EditOperation(getViewer(),
					(DescriptableItem) item, lang, value.toString());
			qmc2Editor.getOperationStack().execute(operation);
		}

	}

}
