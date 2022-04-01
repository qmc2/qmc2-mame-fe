package sourceforge.org.qmc2.options.editor.ui;

import org.eclipse.jface.viewers.ColumnViewer;
import org.eclipse.jface.viewers.ColumnViewerEditorActivationEvent;
import org.eclipse.jface.viewers.ColumnViewerEditorActivationStrategy;
import org.eclipse.swt.SWT;

public class QMC2EditorActivationStrategy extends
		ColumnViewerEditorActivationStrategy {

	public QMC2EditorActivationStrategy(ColumnViewer viewer) {
		super(viewer);
		setEnableEditorActivationWithKeyboard(true);
	}

	@Override
	protected boolean isEditorActivationEvent(
			ColumnViewerEditorActivationEvent event) {
		return event.eventType == ColumnViewerEditorActivationEvent.TRAVERSAL
				|| event.eventType == ColumnViewerEditorActivationEvent.MOUSE_DOUBLE_CLICK_SELECTION
				|| event.eventType == ColumnViewerEditorActivationEvent.PROGRAMMATIC
				|| (event.eventType == ColumnViewerEditorActivationEvent.KEY_PRESSED && (event.keyCode == SWT.CR || event.keyCode == SWT.KEYPAD_CR));
	}
}
