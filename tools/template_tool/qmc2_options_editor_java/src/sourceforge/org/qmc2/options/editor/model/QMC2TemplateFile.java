package sourceforge.org.qmc2.options.editor.model;

import java.io.File;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
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

	private final String emulator;

	private final String version;

	private final String format;

	private final static String TAG_TEMPLATE = "template";

	private final static String ATTRIBUTE_EMULATOR = "emulator";

	private final static String ATTRIBUTE_VERSION = "version";

	private final static String ATTRIBUTE_FORMAT = "format";

	public QMC2TemplateFile(String emulator, String version, String format) {
		this.emulator = emulator;
		this.version = version;
		this.format = format;
	}

	public void addSection(Section section) {
		sections.add(section);
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

		templateFile = new QMC2TemplateFile(emulator, version, format);

		NodeList sections = document.getElementsByTagName(Section.TAG);

		for (int i = 0; i < sections.getLength(); i++) {
			Section s = Section.parseSection(sections.item(i));
			s.setParent(templateFile);
			templateFile.addSection(s);
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

	public void save(File f) throws Exception {
		TransformerFactory transformerFactory = TransformerFactory
				.newInstance();
		Transformer transformer = transformerFactory.newTransformer();
		transformer.setOutputProperty(OutputKeys.INDENT, "yes");
		transformer.setOutputProperty(
				"{http://xml.apache.org/xslt}indent-amount", "2");
		transformer.setOutputProperty(OutputKeys.METHOD, "xml");

		DOMSource source = new DOMSource(toXML());
		FileOutputStream fos = new FileOutputStream(f);

		StreamResult result = new StreamResult(fos);
		transformer.transform(source, result);

		fos.close();
	}

}
