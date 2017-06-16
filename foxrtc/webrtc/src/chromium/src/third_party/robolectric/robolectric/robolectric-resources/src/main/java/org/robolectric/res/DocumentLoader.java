package org.robolectric.res;

import org.jetbrains.annotations.NotNull;
import org.w3c.dom.Document;

import java.io.File;
import java.io.ByteArrayInputStream;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

public class DocumentLoader {
  private static final FsFile.Filter ENDS_WITH_XML = new FsFile.Filter() {
    @Override public boolean accept(@NotNull FsFile fsFile) {
      return fsFile.getName().endsWith(".xml");
    }
  };

  private final FsFile resourceBase;
  private final String packageName;
  private final DocumentBuilderFactory documentBuilderFactory;

  public DocumentLoader(ResourcePath resourcePath) {
    this.resourceBase = resourcePath.resourceBase;
    this.packageName = resourcePath.getPackageName();

    documentBuilderFactory = DocumentBuilderFactory.newInstance();
    documentBuilderFactory.setNamespaceAware(true);
    documentBuilderFactory.setIgnoringComments(true);
    documentBuilderFactory.setIgnoringElementContentWhitespace(true);
  }

  public void load(String folderBaseName, XmlLoader... xmlLoaders) throws Exception {
    FsFile[] files = resourceBase.listFiles(new DirectoryMatchingFilter(folderBaseName));
    if (files == null) {
      throw new RuntimeException(resourceBase.join(folderBaseName) + " is not a directory");
    }
    for (FsFile dir : files) {
      loadFile(dir, xmlLoaders);
    }
  }

  private void loadFile(FsFile dir, XmlLoader[] xmlLoaders) throws Exception {
    if (!dir.exists()) {
      throw new RuntimeException("no such directory " + dir);
    }

    for (FsFile file : dir.listFiles(ENDS_WITH_XML)) {
      loadResourceXmlFile(file, xmlLoaders);
    }
  }

  private void loadResourceXmlFile(FsFile fsFile, XmlLoader... xmlLoaders) throws Exception {
    Document document = parse(fsFile);
    for (XmlLoader xmlLoader : xmlLoaders) {
      xmlLoader.processResourceXml(fsFile, document, packageName);
    }
  }

  private Document parse(FsFile xmlFile) throws Exception {
    DocumentBuilder documentBuilder = documentBuilderFactory.newDocumentBuilder();
    return documentBuilder.parse(new ByteArrayInputStream(xmlFile.getBytes()));
  }

  private static class DirectoryMatchingFilter implements FsFile.Filter {
    private final String folderBaseName;

    public DirectoryMatchingFilter(String folderBaseName) {
      this.folderBaseName = folderBaseName;
    }

    @Override
    public boolean accept(FsFile file) {
      return file.getName().startsWith(folderBaseName);
    }
  }
}
