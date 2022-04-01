package sourceforge.org.qmc2.options.editor.ui.dialogs;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.jface.dialogs.IInputValidator;
import org.eclipse.jface.dialogs.IMessageProvider;
import org.eclipse.jface.dialogs.InputDialog;
import org.eclipse.jface.dialogs.TitleAreaDialog;
import org.eclipse.jface.viewers.ArrayContentProvider;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.window.Window;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.layout.RowData;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

import sourceforge.org.qmc2.options.editor.model.ComboOption;
import sourceforge.org.qmc2.options.editor.model.Option;
import sourceforge.org.qmc2.options.editor.model.Option.OptionType;

public class AddOptionDialog extends TitleAreaDialog {

	private Option option;

	private static final String defaultMessage = "Create a new option in QMC2 template file";

	private Text optionNameText;

	private Text descriptionText;

	private Text defaultValueText;

	private Combo defaultValueCombo;

	private Combo optionType;

	private TableViewer viewer;

	private List<String> choices = new ArrayList<String>();

	public AddOptionDialog(Shell parentShell, Option option) {
		super(parentShell);
		this.option = option;
	}

	@Override
	protected Control createDialogArea(Composite parent) {
		final Composite mainComposite = new Composite(
				(Composite) super.createDialogArea(parent), SWT.NONE);
		mainComposite.setLayoutData(new GridData(GridData.FILL_BOTH));
		mainComposite.setLayout(new GridLayout(3, false));

		Label sectionName = new Label(mainComposite, SWT.NONE);
		sectionName.setText("Name: ");
		sectionName.setLayoutData(new GridData(SWT.LEFT, SWT.TOP, false, false,
				1, 1));

		optionNameText = new Text(mainComposite, SWT.BORDER);
		optionNameText.setLayoutData(new GridData(SWT.FILL, SWT.TOP, true,
				false, 2, 1));
		if (option != null) {
			optionNameText.setText(option.getName());
		}
		optionNameText.addModifyListener(new ModifyListener() {

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
		if (option != null) {
			descriptionText.setText(option.getDescription("us"));
		}
		descriptionText.addModifyListener(new ModifyListener() {

			@Override
			public void modifyText(ModifyEvent arg0) {
				validate();
			}
		});

		Label optionTypeLabel = new Label(mainComposite, SWT.NONE);
		optionTypeLabel.setText("Type: ");
		optionTypeLabel.setLayoutData(new GridData(SWT.LEFT, SWT.TOP, false,
				false, 1, 1));

		optionType = new Combo(mainComposite, SWT.SINGLE | SWT.READ_ONLY);
		optionType.setLayoutData(new GridData(SWT.FILL, SWT.TOP, true, false,
				2, 1));
		String existingType = "";
		if (option != null) {
			existingType = option.getType();
		}
		int index = 0;
		for (OptionType type : OptionType.values()) {
			if (!type.equals(OptionType.UNKNOWN)) {
				String currentType = type.name().toLowerCase();
				optionType.add(currentType);
				if (currentType.equals(existingType)) {
					optionType.select(index);
				}
				index++;
			}
		}
		if (optionType.getSelectionIndex() < 0) {
			optionType.select(0);
		}

		final Composite viewerArea = new Composite(mainComposite, SWT.NONE);
		viewerArea.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true,
				3, 1));
		viewerArea.setLayout(new FillLayout());

		final Label defaultValueLabel = new Label(mainComposite, SWT.NONE);
		defaultValueLabel.setText("Default Value: ");
		defaultValueLabel.setLayoutData(new GridData(SWT.LEFT, SWT.TOP, false,
				false, 1, 1));

		defaultValueText = new Text(mainComposite, SWT.BORDER);
		defaultValueText.setLayoutData(new GridData(SWT.FILL, SWT.TOP, true,
				false, 2, 1));
		if (option != null) {
			defaultValueText.setText(option.getDefaultValue());
		}
		defaultValueText.addModifyListener(new ModifyListener() {

			@Override
			public void modifyText(ModifyEvent arg0) {
				validate();
			}
		});

		final Label defaultValueLabel2 = new Label(mainComposite, SWT.NONE);
		defaultValueLabel2.setText("Default Value: ");
		defaultValueLabel2.setLayoutData(new GridData(SWT.LEFT, SWT.TOP, false,
				false, 1, 1));
		defaultValueLabel2.setVisible(false);

		defaultValueCombo = new Combo(mainComposite, SWT.BORDER | SWT.READ_ONLY
				| SWT.SINGLE);
		defaultValueCombo.setLayoutData(new GridData(SWT.FILL, SWT.TOP, true,
				false, 2, 1));
		if (option != null) {
			defaultValueCombo.setText(option.getDefaultValue());
		}
		defaultValueCombo.addModifyListener(new ModifyListener() {

			@Override
			public void modifyText(ModifyEvent arg0) {
				validate();
			}
		});
		defaultValueCombo.setVisible(false);

		optionType.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				if (optionType.getText().equals(
						OptionType.COMBO.name().toLowerCase())) {
					Composite viewerComposite = new Composite(viewerArea,
							SWT.NONE);
					viewerComposite.setLayout(new GridLayout(2, false));
					createViewer(viewerComposite);
					defaultValueLabel2.setVisible(true);
					defaultValueCombo.setVisible(true);
					defaultValueLabel.setVisible(false);
					defaultValueText.setVisible(false);
					mainComposite.layout(true, true);
					mainComposite.getShell().setSize(
							mainComposite.getShell().computeSize(
									mainComposite.getShell().getSize().x - 4,
									SWT.DEFAULT));
				} else {
					for (Control child : viewerArea.getChildren()) {
						child.dispose();
					}
					defaultValueLabel2.setVisible(false);
					defaultValueCombo.setVisible(false);
					defaultValueLabel.setVisible(true);
					defaultValueText.setVisible(true);
					mainComposite.layout(true, true);
					mainComposite.getShell().setSize(
							mainComposite.getShell().computeSize(
									mainComposite.getShell().getSize().x - 4,
									SWT.DEFAULT));
				}
			}
		});

		setTitle("Add Option");
		setMessage(defaultMessage);

		return mainComposite;
	}

	private void createViewer(Composite parent) {
		viewer = new TableViewer(parent, SWT.V_SCROLL | SWT.H_SCROLL
				| SWT.MULTI | SWT.READ_ONLY | SWT.BORDER);

		viewer.getTable().setLayoutData(
				new GridData(SWT.FILL, SWT.FILL, true, true, 1, 1));

		Composite buttonsComposite = new Composite(parent, SWT.NONE);
		buttonsComposite.setLayoutData(new GridData(SWT.RIGHT, SWT.TOP, false,
				false, 1, 1));
		RowLayout rowLayout = new RowLayout(SWT.VERTICAL);
		rowLayout.pack = false;
		buttonsComposite.setLayout(rowLayout);

		viewer.setLabelProvider(new LabelProvider());
		viewer.setContentProvider(new ArrayContentProvider());
		viewer.setInput(choices);

		Button add = new Button(buttonsComposite, SWT.PUSH);
		add.setText("Add Choice...");
		add.setLayoutData(new RowData());
		add.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				InputDialog choiceDialog = new InputDialog(getShell(),
						"Add choice", "Input choice value", "",
						new IInputValidator() {

							@Override
							public String isValid(String newText) {
								String message = null;
								if (newText.isEmpty()) {
									message = "You must enter a non empty value";
								}
								return message;
							}
						});
				if (choiceDialog.open() == Window.OK) {
					choices.add(choiceDialog.getValue());
					viewer.refresh();
				}
				updateDefaultValueCombo();
			};

		});

		Button remove = new Button(buttonsComposite, SWT.PUSH);
		remove.setText("Remove Choice...");
		remove.setLayoutData(new RowData());
		remove.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				ISelection selection = viewer.getSelection();
				if (selection instanceof IStructuredSelection) {
					IStructuredSelection sselection = (IStructuredSelection) selection;
					for (Object o : sselection.toArray()) {
						choices.remove(o);
					}
					viewer.refresh();
				}
				updateDefaultValueCombo();
			}
		});

	}

	private void updateDefaultValueCombo() {
		String selected = defaultValueCombo.getText();
		defaultValueCombo.removeAll();
		int i = 0;
		for (String choice : choices) {
			defaultValueCombo.add(choice);
			if (choice.equals(selected)) {
				defaultValueCombo.select(i);
			}
			i++;
		}
		if (defaultValueCombo.getSelectionIndex() < 0) {
			defaultValueCombo.select(0);
		}
	}

	private void validate() {
		String errorMessage = null;
		int errorStatus = IMessageProvider.NONE;
		if (optionNameText.getText() == null
				|| optionNameText.getText().trim().length() == 0) {
			errorMessage = "You must enter a valid section name";
			errorStatus = IMessageProvider.ERROR;
		} else if (optionNameText.getText().contains(" ")) {
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
		if (option == null) {
			if (optionType.getText().equals(
					OptionType.COMBO.name().toLowerCase())) {
				option = new ComboOption(optionNameText.getText(),
						optionType.getText(), defaultValueText.getText());
				((ComboOption) option).setChoices(choices);
			} else {
				option = new Option(optionNameText.getText(),
						optionType.getText(), defaultValueText.getText());
			}
		}
		option.setName(optionNameText.getText());
		option.setDescription("us", descriptionText.getText());
		option.setDefaultValue(defaultValueText.getText());
		option.setType(optionType.getText());

		super.okPressed();
	}

	public Option getOption() {
		return option;
	}

}
