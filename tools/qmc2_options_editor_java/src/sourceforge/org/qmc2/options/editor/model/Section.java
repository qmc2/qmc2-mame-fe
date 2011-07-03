package sourceforge.org.qmc2.options.editor.model;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class Section extends DescriptableItem {

	private final List<Option> options = new ArrayList<Option>();

	public final static String TAG_SECTION = "section";

	public Section(String name) {
		super(name);
	}

	public void addOption(Option o) {
		options.add(o);
	}

	public List<Option> getOptions() {
		return options;
	}

	public static Section parseSection(Node sectionNode) {

		String name = sectionNode.getAttributes().getNamedItem(ATTRIBUTE_NAME)
				.getNodeValue();
		Section section = new Section(name);

		section.parseDescriptions(sectionNode);

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

	@Override
	public Element toXML(Document document) {
		Element section = super.toXML(document);
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

}
