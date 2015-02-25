package sourceforge.org.qmc2.options.editor.model;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class Section extends DescriptableItem {

	private final List<Option> options = new ArrayList<Option>();

	public final static String TAG_SECTION = "section";
	
	private Map<String, String> unmappedAttributes = null;

	public Section(String name) {
		super(name);
	}

	public void addOption(Option o, int index) {
		options.add(index, o);
		for (int i = index + 1; i < options.size(); i++) {
			options.get(i).setIndex(i);
		}
	}

	public void addOption(Option o) {
		o.setIndex(options.size());
		options.add(o);
	}

	public List<Option> getOptions() {
		return options;
	}

	public static String parseSectionName(Node sectionNode) {
		String name = null;
		for (int i = 0; i < sectionNode.getAttributes().getLength(); i++) {
			Node item = sectionNode.getAttributes().item(i);
			if (ATTRIBUTE_NAME.equals(item.getNodeName())) {
				name = item.getNodeValue();
			}
		}
		return name;
	}
	
	public static Section parseSection(Node sectionNode) {

		String name = null;
		Map<String, String> unmappedAttributes = new HashMap<String, String>();
		
		for (int i = 0; i < sectionNode.getAttributes().getLength(); i++) {
			Node item = sectionNode.getAttributes().item(i);
			if (ATTRIBUTE_NAME.equals(item.getNodeName())) {
				name = item.getNodeValue();
			}
			else {
				unmappedAttributes.put(item.getNodeName(), item.getNodeValue());
			}
		}

		Section section = new Section(name);

		section.parseDescriptions(sectionNode);
		section.setUnmappedAttributes(unmappedAttributes);

		NodeList options = sectionNode.getChildNodes();

		for (int i = 0; i < options.getLength(); i++) {
			Node option = options.item(i);
			if (Option.TAG_OPTION.equals(option.getNodeName())) {
				Option o = Option.parseOption(option);
				o.setParent(section);
				section.addOption(o);
			}
		}

		return section;
	}

	public void setUnmappedAttributes(Map<String, String> unmappedAttributes) {
		this.unmappedAttributes = unmappedAttributes;
	}

	@Override
	public Element toXML(Document document) {
		Element section = super.toXML(document);
		for (String attr: unmappedAttributes.keySet()) {
			section.setAttribute(attr, unmappedAttributes.get(attr));
		}
		for (Option o : options) {
			section.appendChild(o.toXML(document));
		}
		return section;

	}

	@Override
	public String getTagName() {
		return TAG_SECTION;
	}

	@Override
	public Set<String> getLanguages() {
		Set<String> languages = new HashSet<String>();
		languages.addAll(super.getLanguages());
		for (Option o : options) {
			languages.addAll(o.getLanguages());
		}
		return super.getLanguages();
	}

	public void removeOption(String name) {
		List<Option> optionsCopy = new ArrayList<Option>(options);
		for (Option o : optionsCopy) {
			if (o.getName().equals(name)) {
				options.remove(o);
			}
		}
		for (int i = 0; i < options.size(); i++) {
			options.get(i).setIndex(i);
		}

	}

}
