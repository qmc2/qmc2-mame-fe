package sourceforge.org.qmc2.options.editor.model;

public class Description {
	private final String language;
	private String description;

	public Description(String language, String description) {
		this.language = language;
		this.description = description;
	}

	public String getDescription() {
		return description;
	}

	public void setDescription(String description) {
		this.description = description;
	}

	public String getLanguage() {
		return language;
	}

}
