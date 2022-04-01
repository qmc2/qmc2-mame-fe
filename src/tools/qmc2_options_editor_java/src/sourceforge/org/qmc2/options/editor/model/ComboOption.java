package sourceforge.org.qmc2.options.editor.model;

import java.util.ArrayList;
import java.util.List;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class ComboOption extends Option {

	private final static String TAG_CHOICE = "choice";

	private List<String> choices = new ArrayList<String>();

	public ComboOption(String name, String type, String defaultValue) {
		super(name, type, defaultValue);
	}

	public void parseChoices(Node optionNode) {
		NodeList children = optionNode.getChildNodes();

		for (int i = 0; i < children.getLength(); i++) {
			Node node = children.item(i);
			if (TAG_CHOICE.equals(node.getNodeName())) {
				String choice = node.getAttributes()
						.getNamedItem(ATTRIBUTE_NAME).getNodeValue();
				choices.add(choice);
			}
		}
	}

	@Override
	protected void parseData(Node optionNode) {
		super.parseData(optionNode);
		parseChoices(optionNode);
	}

	@Override
	public Element toXML(Document document) {

		Element option = super.toXML(document);

		for (String choice : choices) {
			Element choiceElement = document.createElement(TAG_CHOICE);
			choiceElement.setAttribute(ATTRIBUTE_NAME, choice);
			option.appendChild(choiceElement);
		}
		return option;
	}

	public void setChoices(List<String> choices) {
		this.choices = choices;
	}

	public List<String> getChoices() {
		return choices;
	}

	public void addChoice(String choice) {
		if (!choices.contains(choice)) {
			choices.add(choice);
		}
	}

}
