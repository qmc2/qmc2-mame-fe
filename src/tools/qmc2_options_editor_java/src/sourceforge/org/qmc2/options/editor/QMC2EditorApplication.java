package sourceforge.org.qmc2.options.editor;

import java.util.HashMap;
import java.util.Map;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.action.IContributionItem;
import org.eclipse.jface.action.IMenuListener;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.jface.window.ApplicationWindow;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;
import sourceforge.org.qmc2.options.editor.ui.actions.AddLanguageAction;
import sourceforge.org.qmc2.options.editor.ui.actions.AddOptionAction;
import sourceforge.org.qmc2.options.editor.ui.actions.AddSectionAction;
import sourceforge.org.qmc2.options.editor.ui.actions.RedoAction;
import sourceforge.org.qmc2.options.editor.ui.actions.RemoveSelectedItemsAction;
import sourceforge.org.qmc2.options.editor.ui.actions.SaveAction;
import sourceforge.org.qmc2.options.editor.ui.actions.UndoAction;

public class QMC2EditorApplication extends ApplicationWindow {

	private final Map<String, IMenuManager> menuMap = new HashMap<String, IMenuManager>();

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
		return manager;
	}

	@Override
	protected Control createContents(Composite parent) {

		QMC2Editor editor = new QMC2Editor(parent, this);

		createActions(editor);

		return editor;
	}

	private void createActions(QMC2Editor editor) {

		IMenuManager fileMenuManager = new MenuManager("&File", MENU_FILE_ID);
		fileMenuManager.addMenuListener(new IMenuListener() {
			@Override
			public void menuAboutToShow(IMenuManager manager) {
				manager.update(IAction.ENABLED);
			}
		});
		IMenuManager editMenuManager = new MenuManager("&Edit", MENU_EDIT_ID);
		editMenuManager.addMenuListener(new IMenuListener() {
			@Override
			public void menuAboutToShow(IMenuManager manager) {
				manager.update(IAction.ENABLED);
			}
		});

		fileMenuManager.add(new SaveAction(editor));
		editMenuManager.add(new UndoAction(editor));
		editMenuManager.add(new RedoAction(editor));
		editMenuManager.add(new AddLanguageAction(editor));
		editMenuManager.add(new AddSectionAction(editor));
		editMenuManager.add(new AddOptionAction(editor));
		editMenuManager.add(new RemoveSelectedItemsAction(editor));

		menuMap.put(MENU_FILE_ID, fileMenuManager);
		menuMap.put(MENU_EDIT_ID, editMenuManager);
		getMenuBarManager().add(fileMenuManager);
		getMenuBarManager().add(editMenuManager);
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
		window.getShell().setText("QMC2 Options Editor");
		window.open();
	}

}
