import java.io.*;

public class CartoonWrapper {
    public static void main(String[] args) throws Exception {
        File outputDirectory = new File("output/");
        if (!outputDirectory.isDirectory()) {
            System.out.println("Output directory must be a valid directory, not " + outputDirectory.toString());
            System.exit(1);
        }
	String[] texts = args;
        String[] nicks = {}; // these seem to get ignored anyway?

	boolean result = ComicTest.createCartoonStrip(outputDirectory, texts, nicks);
    }
}
