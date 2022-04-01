package sourceforge.org.qmc2.options.editor.ui.dialogs;

import org.eclipse.jface.dialogs.IMessageProvider;
import org.eclipse.jface.dialogs.TitleAreaDialog;
import org.eclipse.jface.viewers.ColumnLabelProvider;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredContentProvider;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.viewers.TableViewerColumn;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.window.Window;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.ControlListener;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.layout.RowData;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

import sourceforge.org.qmc2.options.editor.model.Option;
import sourceforge.org.qmc2.options.editor.model.Section;

public class AddSectionDialog extends TitleAreaDialog {

	private Section section;

	private static final String defaultMessage = "Create a new section in QMC2 template file";

	private Text sectionNameText;

	private Text descriptionText;

	private TableViewer viewer;

	public AddSectionDialog(Shell parentShell, Section section) {
		super(parentShell);
		if (section != null) {
			this.section = section;
		} else {
			this.section = new Section(null);
		}
	}

	public Section getSection() {
		return section;
	}

	@Override
	protected Control createDialogArea(Composite parent) {

		Composite mainComposite = new Composite(
				(Composite) super.createDialogArea(parent), SWT.NONE);
		mainComposite.setLayoutData(new GridData(GridData.FILL_BOTH));
		mainComposite.setLayout(new GridLayout(3, false));

		Label sectionName = new Label(mainComposite, SWT.NONE);
		sectionName.setText("Name: ");
		sectionName.setLayoutData(new GridData(SWT.LEFT, SWT.TOP, false, false,
				1, 1));

		sectionNameText = new Text(mainComposite, SWT.BORDER);
		sectionNameText.setLayoutData(new GridData(SWT.FILL, SWT.TOP, true,
				false, 2, 1));
		if (section.getName() != null) {
			sectionNameText.setText(section.getName());
		}
		sectionNameText.addModifyListener(new ModifyListener() {

			@Override
			public void modifyText(ModifyEvent arg0) {
				validate();
			}
		});

		Label description = new Label(mainComposite, SWT.NONE);
		description.setText("Description: ");
		description.setLayoutData(new GridData(SWT.LEFT, SWT.TOP, false, false,
				1, 1));

		descriptionText = new Text(mainComposite, SWT.BORDER);
		descriptionText.setLayoutData(new GridData(SWT.FILL, SWT.TOP, true,
				false, 2, 1));
		if (section.getDescription("us") != null) {
			descriptionText.setText(section.getDescription("us"));
		}
		descriptionText.addModifyListener(new ModifyListener() {

			@Override
			public void modifyText(ModifyEvent arg0) {
				validate();
			}
		});

		createOptionsSection(mainComposite);

		setTitle("Add Section");
		setMessage(defaultMessage);

		return mainComposite;
	}

	private void createOptionsSection(Composite parent) {
		viewer = new TableViewer(parent, SWT.V_SCROLL | SWT.H_SCROLL
				| SWT.MULTI | SWT.READ_ONLY | SWT.BORDER);

		viewer.getTable().setLayoutData(
				new GridData(SWT.FILL, SWT.FILL, true, true, 2, 1));
		viewer.getTable().setHeaderVisible(true);
		viewer.getTable().setLinesVisible(true);
		viewer.setContentProvider(new OptionContentProvider());

		final TableViewerColumn optionNameColumn = new TableViewerColumn(
				viewer, SWT.NONE);
		optionNameColumn.getColumn().setMoveable(false);
		optionNameColumn.getColumn().setText("Name");
		optionNameColumn.setLabelProvider(new OptionNameLabelProvider());

		final TableViewerColumn optionTypeColumn = new TableViewerColumn(
				viewer, SWT.NONE);
		optionTypeColumn.getColumn().setMoveable(false);
		optionTypeColumn.getColumn().setText("Type");
		optionTypeColumn.setLabelProvider(new OptionTypeLabelProvider());

		final TableViewerColumn defaultValueColumn = new TableViewerColumn(
				viewer, SWT.NONE);
		defaultValueColumn.getColumn().setMoveable(false);
		defaultValueColumn.getColumn().setText("Default value");
		defaultValueColumn
				.setLabelProvider(new OptionDefaultValueLabelProvider());

		final TableViewerColumn optionDescriptionColumn = new TableViewerColumn(
				viewer, SWT.NONE);
		optionDescriptionColumn.getColumn().setMoveable(false);
		optionDescriptionColumn.getColumn().setText("Description");
		optionDescriptionColumn
				.setLabelProvider(new OptionDescriptionLabelProvider());

		viewer.getTable().addControlListener(new ControlListener() {

			@Override
			public void controlResized(ControlEvent arg0) {
				optionNameColumn.getColumn().setWidth(
						viewer.getTable().getSize().x / 6);
				optionTypeColumn.getColumn().setWidth(
						viewer.getTable().getSize().x / 6);
				optionDescriptionColumn.getColumn().setWidth(
						viewer.getTable().getSize().x / 3);
				defaultValueColumn.getColumn().setWidth(
						viewer.getTable().getSize().x / 3 - 10);
			}

			@Override
			public void controlMoved(ControlEvent arg0) {
				// do nothing
			}
		});

		Composite buttonsComposite = new Composite(parent, SWT.NONE);
		buttonsComposite.setLayoutData(new GridData(SWT.RIGHT, SWT.TOP, false,
				false, 1, 1));
		RowLayout rowLayout = new RowLayout(SWT.VERTICAL);
		rowLayout.pack = false;
		buttonsComposite.setLayout(rowLayout);

		Button add = new Button(buttonsComposite, SWT.PUSH);
		add.setText("Add Option...");
		add.setLayoutData(new RowData());
		add.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				AddOptionDialog option = new AddOptionDialog(getShell(), null);
				if (option.open() == Window.OK) {
					section.addOption(option.getOption());
					viewer.refresh();
				}
			}
		});

		Button remove = new Button(buttonsComposite, SWT.PUSH);
		remove.setText("Remove Option...");
		remove.setLayoutData(new RowData());
		remove.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				ISelection selection = viewer.getSelection();
				if (selection instanceof IStructuredSelection) {
					IStructuredSelection sselection = (IStructuredSelection) selection;
					for (Object o : sselection.toArray()) {
						section.removeOption(((Option) o).getName());
					}

					viewer.refresh();
				}
			}
		});

		Button edit = new Button(buttonsComposite, SWT.PUSH);
		edit.setText("Edit Option...");
		edit.setLayoutData(new RowData());
		edit.setEnabled(false);

		viewer.setInput(section);
	}

	private void validate() {
		String errorMessage = null;
		int errorStatus = IMessageProvider.NONE;
		if (sectionNameText.getText() == null
				|| sectionNameText.getText().trim().length() == 0) {
			errorMessage = "You must enter a valid section name";
			errorStatus = IMessageProvider.ERROR;
		} else if (sectionNameText.getText().contains(" ")) {
			errorMessage = "The section name must not contain whitespaces";
			errorStatus = IMessageProvider.ERROR;
		}

		if (errorMessage == null
				&& (descriptionText.getText() == null || descriptionText
						.getText().trim().length() == 0)) {
			errorMessage = "You must enter a valid description";
			errorStatus = IMessageProvider.ERROR;
		}

		setMessage(errorStatus == IMessageProvider.ERROR ? errorMessage
				: defaultMessage, errorStatus);
		if (getButton(OK) != null) {
			getButton(OK).setEnabled(errorStatus != IMessageProvider.ERROR);
		}

	}

	@Override
	protected boolean isResizable() {
		return true;
	}

	@Override
	protected void okPressed() {
		section.setName(sectionNameText.getText());
		section.setDescription("us", descriptionText.getText());
		super.okPressed();
	}

	private class OptionContentProvider implements IStructuredContentProvider {

		@Override
		public void dispose() {
			// do nothing
		}

		@Override
		public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {
			// do nothing
		}

		@Override
		public Object[] getElements(Object inputElement) {
			return section.getOptions().toArray(new Option[0]);
		}
	}

	private class OptionNameLabelProvider extends ColumnLabelProvider {
		@Override
		public String getText(Object element) {
			return ((Option) element).getName();
		}
	}

	private class OptionTypeLabelProvider extends ColumnLabelProvider {
		@Override
		public String getText(Object element) {
			return ((Option) element).getType();
		}
	}

	private class OptionDescriptionLabelProvider extends ColumnLabelProvider {
		@Override
		public String getText(Object element) {
			return ((Option) element).getDescription("us");
		}
	}

	private class OptionDefaultValueLabelProvider extends ColumnLabelProvider {
		@Override
		public String getText(Object element) {
			return ((Option) element).getDefaultValue();
		}
	}
}
