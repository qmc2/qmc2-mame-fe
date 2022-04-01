package sourceforge.org.qmc2.options.editor.ui;

import java.io.File;
import java.util.Set;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.operations.DefaultOperationHistory;
import org.eclipse.core.commands.operations.IOperationHistory;
import org.eclipse.core.commands.operations.IUndoContext;
import org.eclipse.core.commands.operations.IUndoableOperation;
import org.eclipse.core.commands.operations.UndoContext;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.action.IMenuListener;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ColumnViewerEditor;
import org.eclipse.jface.viewers.FocusCellOwnerDrawHighlighter;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.jface.viewers.TreeViewerColumn;
import org.eclipse.jface.viewers.TreeViewerEditor;
import org.eclipse.jface.viewers.TreeViewerFocusCellManager;
import org.eclipse.jface.window.ApplicationWindow;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.TreeColumn;

import sourceforge.org.qmc2.options.editor.model.QMC2TemplateFile;
import sourceforge.org.qmc2.options.editor.ui.actions.AddLanguageAction;
import sourceforge.org.qmc2.options.editor.ui.actions.AddOptionAction;
import sourceforge.org.qmc2.options.editor.ui.actions.AddSectionAction;
import sourceforge.org.qmc2.options.editor.ui.actions.RedoAction;
import sourceforge.org.qmc2.options.editor.ui.actions.RemoveSelectedItemsAction;
import sourceforge.org.qmc2.options.editor.ui.actions.UndoAction;

public class QMC2Editor extends Composite {

	private final TreeViewer viewer;

	public static final String EDITOR_ID = "sourceforge.org.qmc2.options.editor";

	private String selectedFile = null;

	private QMC2TemplateFile templateFile = null;

	private String filter = null;

	private IOperationHistory operationHistory = new DefaultOperationHistory();

	private IUndoContext undoContext = new UndoContext();

	private final ApplicationWindow application;

	public QMC2Editor(Composite parent, ApplicationWindow app) {
		super(parent, SWT.NONE);
		this.application = app;
		setLayout(new GridLayout(3, false));

		createFileChooser();
		createFilterAndSearch();

		viewer = new TreeViewer(this, SWT.MULTI | SWT.V_SCROLL | SWT.H_SCROLL
				| SWT.BORDER | SWT.FULL_SELECTION);
		viewer.setContentProvider(new QMC2ContentProvider());
		viewer.getTree().setHeaderVisible(true);
		viewer.getTree().setLinesVisible(true);
		viewer.getTree().setLayoutData(
				new GridData(SWT.FILL, SWT.FILL, true, true, 3, 1));

		TreeViewerEditor.create(viewer, new TreeViewerFocusCellManager(viewer,
				new FocusCellOwnerDrawHighlighter(viewer)),
				new QMC2EditorActivationStrategy(viewer),
				ColumnViewerEditor.TABBING_HORIZONTAL
						| ColumnViewerEditor.TABBING_MOVE_TO_ROW_NEIGHBOR
						| ColumnViewerEditor.TABBING_VERTICAL
						| ColumnViewerEditor.KEYBOARD_ACTIVATION);

		viewer.addFilter(new QMC2ViewFilter(this));

		MenuManager manager = new MenuManager();
		manager.setRemoveAllWhenShown(true);
		manager.addMenuListener(new IMenuListener() {
			@Override
			public void menuAboutToShow(IMenuManager manager) {
				populateContextMenu(manager);
			}
		});
		Menu m = manager.createContextMenu(viewer.getControl());
		viewer.getTree().setMenu(m);
		viewer.addSelectionChangedListener(new ISelectionChangedListener() {

			@Override
			public void selectionChanged(SelectionChangedEvent event) {
				updateApplicationMenuBar();
			}
		});

	}

	private void populateContextMenu(IMenuManager manager) {
		manager.add(new UndoAction(QMC2Editor.this));
		manager.add(new RedoAction(QMC2Editor.this));
		manager.add(new AddLanguageAction(QMC2Editor.this));
		manager.add(new AddSectionAction(QMC2Editor.this));
		manager.add(new AddOptionAction(QMC2Editor.this));
		manager.add(new RemoveSelectedItemsAction(QMC2Editor.this));
	}

	private void createFilterAndSearch() {
		GridData layoutData = new GridData(SWT.LEAD, SWT.TOP, false, false, 1,
				1);
		Label filterLabel = new Label(this, SWT.NONE);
		filterLabel.setText("Filter: ");
		filterLabel.setLayoutData(layoutData);

		layoutData = new GridData(SWT.FILL, SWT.TOP, true, false, 2, 1);
		final Text filterText = new Text(this, SWT.SEARCH);
		filterText.setMessage("Filter...");
		filterText.setLayoutData(layoutData);
		filterText.addModifyListener(new ModifyListener() {

			@Override
			public void modifyText(ModifyEvent arg0) {
				setFilter(filterText.getText() == null ? null : filterText
						.getText().trim().length() == 0 ? null : filterText
						.getText().toLowerCase());

				viewer.refresh();

			}
		});

	}

	private void createFileChooser() {

		GridData layoutData = new GridData(SWT.LEAD, SWT.TOP, false, false, 1,
				1);
		Label fileSelectionLabel = new Label(this, SWT.NONE);
		fileSelectionLabel.setText("Template File: ");
		fileSelectionLabel.setLayoutData(layoutData);

		final Text fileSelection = new Text(this, SWT.BORDER | SWT.SINGLE);
		layoutData = new GridData(SWT.FILL, SWT.TOP, true, false, 1, 1);
		fileSelection.setLayoutData(layoutData);
		fileSelection.addModifyListener(new ModifyListener() {

			@Override
			public void modifyText(ModifyEvent e) {
				selectedFile = fileSelection.getText();
				File f = new File(selectedFile);
				if (f.exists() && f.isFile() && f.canRead()) {
					try {
						templateFile = QMC2TemplateFile.parse(new File(
								selectedFile));
						createColumns(templateFile.getLanguages());
						operationHistory = new DefaultOperationHistory();
						viewer.setInput(templateFile);
						updateApplicationMenuBar();
					} catch (Exception e1) {
						MessageDialog.openError(
								getShell(),
								"Error",
								"Unable to open template file: "
										+ e1.getMessage());
					}
				}

			}
		});

		Button browse = new Button(this, SWT.PUSH);
		browse.setText("Browse...");
		layoutData = new GridData(SWT.TRAIL, SWT.TOP, false, false, 1, 1);
		browse.setLayoutData(layoutData);

		browse.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				FileDialog dialog = new FileDialog(getShell(), SWT.OPEN);
				dialog.setFileName(selectedFile);
				String filename = dialog.open();
				if (filename != null) {
					selectedFile = filename;
					fileSelection.setText(filename);
				}
			}
		});

	}

	private void createColumns(Set<String> languages) {

		int KEYS_COLUMN_SIZE = 150;

		viewer.getTree().setRedraw(false);
		for (TreeColumn c : viewer.getTree().getColumns()) {
			c.dispose();
		}

		TreeViewerColumn c = new TreeViewerColumn(viewer, SWT.NONE);

		c.getColumn().setMoveable(false);
		c.getColumn().setText("Item");
		c.setLabelProvider(new QMC2LabelProvider(null));
		c.getColumn().setWidth(KEYS_COLUMN_SIZE);

		for (String lang : languages) {
			c = createColumn(viewer, lang, "us".equals(lang) ? 1 : -1);
		}

		viewer.getTree().setRedraw(true);
		int columnSize = (viewer.getTree().getSize().x - KEYS_COLUMN_SIZE)
				/ (viewer.getTree().getColumnCount() - 1);

		for (int i = 1; i < viewer.getTree().getColumnCount(); i++) {
			viewer.getTree().getColumn(i).setWidth(columnSize);
		}

	}

	public TreeViewerColumn createColumn(TreeViewer viewer, String lang,
			int columnIndex) {
		TreeViewerColumn c = new TreeViewerColumn(viewer, SWT.NONE, columnIndex);
		c.getColumn().setMoveable(false);
		c.getColumn().setText(lang);
		c.setLabelProvider(new QMC2LabelProvider(lang));
		c.setEditingSupport(new QMC2EditingSupport(this, lang));
		return c;
	}

	public TreeViewer getViewer() {
		return viewer;
	}

	public IOperationHistory getOperationHistory() {
		return operationHistory;
	}

	public String getCurrentFile() {
		return selectedFile;
	}

	public QMC2TemplateFile getTemplateFile() {
		return templateFile;
	}

	public IUndoContext getUndoContext() {
		return undoContext;
	}

	public String getFilter() {
		return filter;
	}

	public void setFilter(String filter) {
		this.filter = filter;
	}

	public void updateApplicationMenuBar() {
		application.getMenuBarManager().update(IAction.TEXT);
		application.getMenuBarManager().update(IAction.ENABLED);
	}

	public void executeOperation(IUndoableOperation operation) {
		operation.addContext(undoContext);
		try {
			operationHistory
					.execute(operation, new NullProgressMonitor(), null);
		} catch (ExecutionException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}
