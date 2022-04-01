package sourceforge.org.qmc2.options.editor.model;

import java.io.File;
import java.io.FileOutputStream;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class QMC2TemplateFile {

	private final List<Section> sections = new ArrayList<Section>();

	private final Map<String, Section> sectionMap = new HashMap<String, Section>();
	
	private final List<Section> addedSections = new ArrayList<Section>();
	
	private final List<Section> removedSections = new ArrayList<Section>();

	// we should not rewrite whole xml to keep comments and other possible formatting
	private final Document document;
	private final Node rootNode;
	
	private final String emulator;

	private final String version;

	private final String format;

	private final static String TAG_TEMPLATE = "template";

	private final static String ATTRIBUTE_EMULATOR = "emulator";

	private final static String ATTRIBUTE_VERSION = "version";

	private final static String ATTRIBUTE_FORMAT = "format";
	
	public QMC2TemplateFile(Document document, String emulator, String version, String format) {
		this.document = document;
		this.rootNode = document.getElementsByTagName(TAG_TEMPLATE).item(0);
		this.emulator = emulator;
		this.version = version;
		this.format = format;
	}

	public void addSectionInternal(Section section) {
		section.setIndex(sections.size());
		sections.add(section);
		sectionMap.put(section.getName(), section);
	}
	
	public void addSection(Section section) {
		addSectionInternal(section);
		if (!removedSections.remove(section)) {
			addedSections.add(section);
		}
	}

	public void addSection(Section section, int index) {
		sections.add(index, section);
		for (int i = index + 1; i < sections.size(); i++) {
			sections.get(i).setIndex(i);
		}
		sectionMap.put(section.getName(), section);
		if (!removedSections.remove(section)) {
			addedSections.add(section);
		}
	}

	public Section removeSection(String sectionName) {
		Section s = sectionMap.remove(sectionName);
		sections.remove(s);
		for (int i = 0; i < sections.size(); i++) {
			sections.get(i).setIndex(i);
		}
		if (!addedSections.remove(s)) {
			removedSections.add(s);	
		}
		return s;
	}

	public List<Section> getSections() {
		return sections;
	}

	public static QMC2TemplateFile parse(File file) throws Exception {

		QMC2TemplateFile templateFile = null;
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		DocumentBuilder builder = factory.newDocumentBuilder();
		Document document = builder.parse(file);

		Node templateNode = document.getElementsByTagName(TAG_TEMPLATE).item(0);

		String emulator = templateNode.getAttributes()
				.getNamedItem(ATTRIBUTE_EMULATOR).getNodeValue();
		String version = templateNode.getAttributes()
				.getNamedItem(ATTRIBUTE_VERSION).getNodeValue();
		String format = templateNode.getAttributes()
				.getNamedItem(ATTRIBUTE_FORMAT).getNodeValue();

		templateFile = new QMC2TemplateFile(document, emulator, version, format);

		NodeList sections = document.getElementsByTagName(Section.TAG_SECTION);

		for (int i = 0; i < sections.getLength(); i++) {
			Section s = Section.parseSection(sections.item(i));
			s.setParent(templateFile);
			templateFile.addSectionInternal(s);
		}
		return templateFile;
	}

	public Set<String> getLanguages() {

		Set<String> languages = new HashSet<String>();
		for (Section s : sections) {
			languages.addAll(s.getLanguages());
		}
		return languages;
	}

	private Document toXML() throws Exception {
		DocumentBuilderFactory documentBuilderFactory = DocumentBuilderFactory
				.newInstance();
		DocumentBuilder documentBuilder;
		Document document = null;

		documentBuilder = documentBuilderFactory.newDocumentBuilder();
		document = documentBuilder.newDocument();
		Element rootElement = document.createElement(TAG_TEMPLATE);
		rootElement.setAttribute(ATTRIBUTE_EMULATOR, emulator);
		rootElement.setAttribute(ATTRIBUTE_VERSION, version);
		rootElement.setAttribute(ATTRIBUTE_FORMAT, format);

		for (Section s : sections) {
			rootElement.appendChild(s.toXML(document));
		}
		document.appendChild(rootElement);
		return document;
	}
	
	private Node findSectionInDocument(String name) {
		Node section = null;
		NodeList sections = document.getElementsByTagName(Section.TAG_SECTION);
		int i = 0;
		while (i < sections.getLength() && section == null) {
			Node candidate = sections.item(i);
			String sectionName = Section.parseSectionName(candidate);
			if (name.equals(sectionName)) {
				section = candidate;
			}
			i++;
		}
		
		return section;
	}
	
	private Node getCommentNodeForSection(Node node) {
		Node commentNode = null;
		Node previousNode = node.getPreviousSibling();
		
		while (previousNode != null &&
				!previousNode.getNodeName().equals(Section.TAG_SECTION))
		{
			if (previousNode.getNodeType() == Node.COMMENT_NODE)
			{
				commentNode = previousNode;
			}
			previousNode = previousNode.getPreviousSibling();
		}
		
		return commentNode;
	}
	
	private void processRemovedSections() {
		for (Section section : removedSections) {
			Node sectionElement = findSectionInDocument(section.getName());
			if (sectionElement != null) {
				Node commentNode = getCommentNodeForSection(sectionElement);
				List<Node> nodesToRemove = new ArrayList<Node>();
				nodesToRemove.add(sectionElement);
				if (commentNode != null) {
					Node aux = sectionElement.getPreviousSibling();
					while (aux != commentNode) {
						nodesToRemove.add(aux);
						aux = aux.getPreviousSibling();
					}
					nodesToRemove.add(aux);
				}
				
				for (Node node: nodesToRemove) {
					rootNode.removeChild(node);
				}
			}
		}
	}
	
	private void processAddedSections() {
		Collections.sort(addedSections, new Comparator<Section>() {

			@Override
			public int compare(Section o1, Section o2) {
				//sort descending
				if (o1.getIndex() == o2.getIndex()) {
					return 0;
				}
				else if (o1.getIndex() > o2.getIndex()) {
					return -1;
				}
				else {
					return 1;
				}
			}
			
		});
		
		for (Section section : addedSections) {
			NodeList sectionNodes = document.getElementsByTagName(Section.TAG_SECTION);
			Node newChild = section.toXML(document);
						
			if (section.getIndex() < sectionNodes.getLength()) {
				Node refChild = sectionNodes.item(section.getIndex());
				Node commentNode = getCommentNodeForSection(refChild);
				rootNode.insertBefore(newChild, commentNode != null ? commentNode : refChild);
			}
			else {
				rootNode.appendChild(newChild);
			}

		}
	}
	
	private void processChangedSections() {
		NodeList sectionNodes = document.getElementsByTagName(Section.TAG_SECTION);
		for (int i = 0; i < sectionNodes.getLength(); i++) {
			Node node = sectionNodes.item(i);
			String sectionName = Section.parseSectionName(node);
			Section section = sectionMap.get(sectionName);
			if (section != null) {
				Node newChild = section.toXML(document);
				rootNode.replaceChild(newChild, node);
			}
		}
	}

	public void save(File f) throws Exception {
		TransformerFactory transformerFactory = TransformerFactory
				.newInstance();
		Transformer transformer = transformerFactory.newTransformer();
		transformer.setOutputProperty(OutputKeys.INDENT, "yes");
		transformer.setOutputProperty(
				"{http://xml.apache.org/xslt}indent-amount", "5");
		transformer.setOutputProperty(OutputKeys.METHOD, "xml");

		processRemovedSections();
		processAddedSections();
		processChangedSections();
		
		DOMSource source = new DOMSource(document);
		StringWriter output = new StringWriter();

		StreamResult result = new StreamResult(output);
		transformer.transform(source, result);

		String outputString = output.toString().replace("     ", "\t");
		FileOutputStream fos = new FileOutputStream(f);
		fos.write(outputString.getBytes());
		fos.close();
	}

}
