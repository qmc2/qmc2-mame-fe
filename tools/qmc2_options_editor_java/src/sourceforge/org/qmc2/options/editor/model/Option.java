package sourceforge.org.qmc2.options.editor.model;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

public class Option extends DescriptableItem {

	private final String type;

	private final String defaultValue;

	public final static String TAG = "option";

	public final static String ATTRIBUTE_TYPE = "type";

	public final static String ATTRIBUTE_DEFAULT = "default";

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

		return option;
	}

	@Override
	public String getTagName() {
		return TAG;
	}

	@Override
	public Element toXML(Document document) {
		Element option = super.toXML(document);
		option.setAttribute(ATTRIBUTE_TYPE, type);
		option.setAttribute(ATTRIBUTE_DEFAULT, defaultValue);

		return option;

	}

}
