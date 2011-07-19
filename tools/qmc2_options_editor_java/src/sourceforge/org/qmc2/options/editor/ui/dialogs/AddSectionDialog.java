package sourceforge.org.qmc2.options.editor.ui.dialogs;

import org.eclipse.jface.dialogs.IMessageProvider;
import org.eclipse.jface.dialogs.TitleAreaDialog;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
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

import sourceforge.org.qmc2.options.editor.model.Section;

public class AddSectionDialog extends TitleAreaDialog {

	private Section section;

	private static final String defaultMessage = "Create a new section in QMC2 template file";

	private Text sectionNameText;

	private Text descriptionText;

	private TableViewer viewer;

	public AddSectionDialog(Shell parentShell) {
		super(parentShell);
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
		descriptionText.addModifyListener(new ModifyListener() {

			@Override
			public void modifyText(ModifyEvent arg0) {
				validate();
			}
		});

		viewer = new TableViewer(mainComposite, SWT.V_SCROLL | SWT.H_SCROLL
				| SWT.MULTI | SWT.READ_ONLY | SWT.BORDER);

		viewer.getTable().setLayoutData(
				new GridData(SWT.FILL, SWT.FILL, true, true, 2, 1));

		Composite buttonsComposite = new Composite(mainComposite, SWT.NONE);
		buttonsComposite.setLayoutData(new GridData(SWT.RIGHT, SWT.TOP, false,
				false, 1, 1));
		RowLayout rowLayout = new RowLayout(SWT.VERTICAL);
		rowLayout.pack = false;
		buttonsComposite.setLayout(rowLayout);

		Button add = new Button(buttonsComposite, SWT.PUSH);
		add.setText("Add Option...");
		add.setLayoutData(new RowData());
		add.setEnabled(false);

		Button remove = new Button(buttonsComposite, SWT.PUSH);
		remove.setText("Remove Option...");
		remove.setLayoutData(new RowData());
		remove.setEnabled(false);

		Button edit = new Button(buttonsComposite, SWT.PUSH);
		edit.setText("Edit Option...");
		edit.setLayoutData(new RowData());
		edit.setEnabled(false);

		setTitle("Add Section");
		setMessage(defaultMessage);

		return mainComposite;
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

	}

	@Override
	protected boolean isResizable() {
		return true;
	}

	@Override
	protected void okPressed() {
		section = new Section(sectionNameText.getText());
		section.setDescription("us", descriptionText.getText());
		super.okPressed();
	}

}
