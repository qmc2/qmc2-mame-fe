package sourceforge.org.qmc2.options.editor.model;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

public class Option extends DescriptableItem {

	public enum OptionType {
		BOOL, COMBO, DIRECTORY, FILE, FLOAT, FLOAT2, FLOAT3, INT, STRING, UNKNOWN
	};

	private String type;

	private String defaultValue;

	public final static String TAG_OPTION = "option";

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

		OptionType optionType = null;
		try {
			optionType = OptionType.valueOf(type.toUpperCase());
		} catch (Exception e) {
			optionType = OptionType.UNKNOWN;
		}

		Option option = null;

		switch (optionType) {
		case COMBO:
			option = new ComboOption(name, optionType.name().toLowerCase(),
					defaultValue);
			break;

		case UNKNOWN:
			option = new Option(name, type, defaultValue);

		default:
			option = new Option(name, optionType.name().toLowerCase(),
					defaultValue);
			break;
		}

		option.parseDescriptions(optionNode);
		option.parseData(optionNode);

		return option;
	}

	protected void parseData(Node optionNode) {

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

		return option;

	}

	public void setDefaultValue(String defaultValue) {
		this.defaultValue = defaultValue;

	}

	public void setType(String newType) {
		this.type = newType;

	}

}
