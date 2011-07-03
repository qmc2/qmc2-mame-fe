package sourceforge.org.qmc2.options.editor.model;

import java.util.ArrayList;
import java.util.List;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class Option extends DescriptableItem {

	private final String type;

	private final String defaultValue;

	private final List<String> choices = new ArrayList<String>();

	public final static String TAG_OPTION = "option";

	private final static String TAG_CHOICE = "choice";

	private final static String ATTRIBUTE_TYPE = "type";

	private final static String ATTRIBUTE_DEFAULT = "default";

	public Option(String name, String type, String defaultValue) {
		super(name);
		this.type = type;
		this.defaultValue = defaultValue;
	}

	public String getType() {
		return type;
	}

	public String getDefaultValue() {
		return defaultValue;
	}

	public static Option parseOption(Node optionNode) {

		String name = optionNode.getAttributes().getNamedItem(ATTRIBUTE_NAME)
				.getNodeValue();
		String type = optionNode.getAttributes().getNamedItem(ATTRIBUTE_TYPE)
				.getNodeValue();
		String defaultValue = optionNode.getAttributes()
				.getNamedItem(ATTRIBUTE_DEFAULT).getNodeValue();

		Option option = new Option(name, type, defaultValue);

		option.parseDescriptions(optionNode);

		option.parseChoices(optionNode);

		return option;
	}

	private void parseChoices(Node optionNode) {
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
	public String getTagName() {
		return TAG_OPTION;
	}

	@Override
	public Element toXML(Document document) {
		Element option = super.toXML(document);
		option.setAttribute(ATTRIBUTE_TYPE, type);
		option.setAttribute(ATTRIBUTE_DEFAULT, defaultValue);

		for (String choice : choices) {
			Element choiceElement = document.createElement(TAG_CHOICE);
			choiceElement.setAttribute(ATTRIBUTE_NAME, choice);
			option.appendChild(choiceElement);
		}

		return option;

	}

}
