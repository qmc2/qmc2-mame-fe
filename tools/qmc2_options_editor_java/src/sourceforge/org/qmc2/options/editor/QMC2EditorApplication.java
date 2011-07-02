package sourceforge.org.qmc2.options.editor;

import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class QMC2EditorApplication {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Display display = new Display();
		Shell shell = new Shell(display);
		new QMC2Editor(shell);
		shell.setLayout(new FillLayout());
		shell.setSize(800, 600);
		shell.open();
		while (!shell.isDisposed()) {
			if (!display.readAndDispatch())
				display.sleep();
		}
		display.dispose();
	}

}
