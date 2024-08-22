# pdf-pages

A simple library for working with PDF pages, implementing the PDF ISO standard down to the page level.

## Capabilities

- **Extract Specific Pages**: Allows you to extract specific pages from a PDF file.
- **Merge PDF Files**: Supports merging multiple PDF files into a single document.

## Examples

- Refer to `example.cpp` and `merge.cpp` for usage examples.

## Warning

This project is intended **only for learning** purposes, focusing on various parsing, optimization, and serialization techniques. It is very bare-bones and **should not be used** for any serious use cases. 

### Limitations:

- Not fully compliant with the PDF standard.
- Removes all annotations from PDFs.

## Planned Improvements

- **Lazy Parsing**: Implement lazy parsing with utilization of the cross-reference table.
- **Avoid Rendering Duplicate Objects**: Optimize the rendering process by avoiding duplicate objects.
- **Balanced Pages Tree**: Construct a balanced pages tree for faster loading.
- **Support for Incrementally Updated PDFs**: Add support for incrementally updated PDFs.
- **Advanced PDF Features**: Extend support to include more advanced PDF features.

---

**Note**: This project is still under development and is far from being production-ready.
