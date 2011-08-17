package sourceforge.org.qmc2.options.editor.ui;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.operations.IUndoableOperation;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.jface.viewers.CellEditor;
import org.eclipse.jface.viewers.EditingSupport;
import org.eclipse.jface.viewers.TextCellEditor;
import org.eclipse.swt.widgets.Composite;

import sourceforge.org.qmc2.options.editor.model.DescriptableItem;
import sourceforge.org.qmc2.options.editor.ui.operations.EditOperation;

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

		return value == null ? "" : value;
	}

	@Override
	protected void setValue(Object item, Object value) {
		if (item instanceof DescriptableItem) {
			String oldValue = ((DescriptableItem) item).getDescription(lang);
			if ((value != null && oldValue != null && !oldValue.equals(value
					.toString()))
					|| (value != null && oldValue == null)
					|| (value == null && oldValue != null)) {
				IUndoableOperation operation = new EditOperation(qmc2Editor,
						(DescriptableItem) item, lang, value.toString());
				try {
					qmc2Editor.getOperationHistory().execute(operation,
							new NullProgressMonitor(), null);
				} catch (ExecutionException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}

			}

		}
		qmc2Editor.updateApplicationMenuBar();

	}
}
