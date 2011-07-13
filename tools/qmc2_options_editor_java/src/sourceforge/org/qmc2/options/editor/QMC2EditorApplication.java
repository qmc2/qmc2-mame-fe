package sourceforge.org.qmc2.options.editor;

import org.eclipse.jface.action.IContributionItem;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.jface.window.ApplicationWindow;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class QMC2EditorApplication extends ApplicationWindow {

	public static final String MENU_FILE_ID = "file";

	public static final String MENU_EDIT_ID = "edit";

	public QMC2EditorApplication(Shell parentShell) {
		super(parentShell);
		addMenuBar();
		create();
		getMenuBarManager().updateAll(true);

	}

	public IMenuManager getMenuManager(String id) {
		IMenuManager manager = null;
		for (IContributionItem item : getMenuBarManager().getItems()) {
			if (item instanceof IMenuManager) {
				if (item.getId().equals(id)) {
					manager = (IMenuManager) item;
					break;
				}
			}
		}
		return manager;
	}

	@Override
	protected MenuManager createMenuManager() {
		MenuManager manager = super.createMenuManager();
		manager.add(new MenuManager("&File", MENU_FILE_ID));
		manager.add(new MenuManager("&Edit", MENU_EDIT_ID));
		return manager;
	}

	@Override
	protected Control createContents(Composite parent) {
		return new QMC2Editor(parent, this);
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Display display = new Display();
		Shell shell = new Shell(display);
		shell.setLayout(new FillLayout());
		ApplicationWindow window = new QMC2EditorApplication(shell);
		window.setBlockOnOpen(true);
		window.getShell().setSize(800, 600);
		window.open();
	}

}
