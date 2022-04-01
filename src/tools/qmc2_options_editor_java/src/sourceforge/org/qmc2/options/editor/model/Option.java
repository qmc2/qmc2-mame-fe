package sourceforge.org.qmc2.options.editor.model;

import java.util.HashMap;
import java.util.Map;

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

	private Map<String, String> unmappedAttributes;
	
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

		String name = null;
		String type = null;
		String defaultValue = null;
		Map<String, String> unmappedAttrs = new HashMap<String, String>();
		for (int i = 0; i < optionNode.getAttributes().getLength(); i++) {
			Node item = optionNode.getAttributes().item(i);
			if (ATTRIBUTE_NAME.equals(item.getNodeName())) {
				name = item.getNodeValue();
			}
			else if(ATTRIBUTE_TYPE.equals(item.getNodeName())){
				type = item.getNodeValue();
			}
			else if(ATTRIBUTE_DEFAULT.equals(item.getNodeName())) {
				defaultValue = item.getNodeValue();
			}
			else {
				unmappedAttrs.put(item.getNodeName(), item.getNodeValue());
			}
		}
		
		

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
		option.setUnmappedAttributes(unmappedAttrs);

		return option;
	}

	protected void parseData(Node optionNode) {

	}
	
	
	public void setUnmappedAttributes(Map<String, String> unmappedAttributes) {
		this.unmappedAttributes = unmappedAttributes;
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
		for (String attr: unmappedAttributes.keySet()) {
			option.setAttribute(attr, unmappedAttributes.get(attr));
		}
		

		return option;

	}

	public void setDefaultValue(String defaultValue) {
		this.defaultValue = defaultValue;

	}

	public void setType(String newType) {
		this.type = newType;

	}

}
