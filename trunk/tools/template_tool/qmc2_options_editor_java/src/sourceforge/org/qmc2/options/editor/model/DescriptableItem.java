package sourceforge.org.qmc2.options.editor.model;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public abstract class DescriptableItem {

	private final Map<String, Description> descriptionsMap = new HashMap<String, Description>();

	private final List<Description> descriptions = new ArrayList<Description>();

	private static final String TAG_DESCRIPTION = "description";

	protected static final String ATTRIBUTE_NAME = "name";

	private static final String ATTRIBUTE_LANG = "lang";

	private static final String ATTRIBUTE_TEXT = "text";

	private Object parent = null;

	private final String name;

	public DescriptableItem(String name) {
		this.name = name;
	}

	public String getName() {
		return name;
	}

	public String getDescription(String language) {
		return descriptionsMap.get(language) != null ? descriptionsMap.get(
				language).getDescription() : null;
	}

	public void setDescription(String language, String value) {
		Description descr = descriptionsMap.get(language);
		if (descr == null) {
			descr = new Description(language, null);
			descriptionsMap.put(language, descr);
			descriptions.add(descr);
		}
		descr.setDescription(value);

	}

	public Set<String> getLanguages() {
		return descriptionsMap.keySet();
	}

	public Object getParent() {
		return parent;
	}

	public void setParent(Object parent) {
		this.parent = parent;
	}

	public void parseDescriptions(Node parentNode) {
		NodeList children = parentNode.getChildNodes();

		for (int i = 0; i < children.getLength(); i++) {
			Node node = children.item(i);
			if (TAG_DESCRIPTION.equals(node.getNodeName())) {
				String language = node.getAttributes()
						.getNamedItem(ATTRIBUTE_LANG).getNodeValue();
				String text = node.getAttributes().getNamedItem(ATTRIBUTE_TEXT)
						.getNodeValue();
				Description descr = new Description(language, text);
				descriptionsMap.put(language, descr);
				descriptions.add(descr);
			}
		}

	}

	public Element toXML(Document document) {
		Element descriptableItem = document.createElement(getTagName());
		descriptableItem.setAttribute(ATTRIBUTE_NAME, name);

		for (Description description : descriptions) {
			Element descriptionElement = document
					.createElement(TAG_DESCRIPTION);
			descriptionElement.setAttribute(ATTRIBUTE_LANG,
					description.getLanguage());
			descriptionElement.setAttribute(ATTRIBUTE_TEXT,
					description.getDescription());
			descriptableItem.appendChild(descriptionElement);
		}

		return descriptableItem;
	}

	public abstract String getTagName();

}
