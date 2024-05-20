/**
 * Applies a fix for CORS (https://github.com/CrowCpp/Crow/pull/771/files) on crow library
 */

const fs = require("fs/promises");

const NEWLINE = "\n";
const FILE_FORMAT = "utf8";

const CROW_INCLUDE_PATH = "./backend/vcpkg_installed/x64-windows/x64-windows/include/crow/";
const CROW_VERSION_TO_FIX = "1.1.0";
const LINE_TO_APPEND = 106;
const CODE_TO_APPEND = "                need_to_call_after_handlers_ = true;";

async function main() {
  const versionPath = `${CROW_INCLUDE_PATH}/version.h`;
  const connectionPath = `${CROW_INCLUDE_PATH}/http_connection.h`;

  try {
    const versionContent = await fs.readFile(versionPath, FILE_FORMAT);
    if (!versionContent.includes(CROW_VERSION_TO_FIX)) {
      console.log(`NOTE: Crow version is not ${CROW_VERSION_TO_FIX}, so fix is probably not needed anymore! Skipping...`);
      return;
    }

    const content = await fs.readFile(connectionPath, FILE_FORMAT);

    const lines = content.split(NEWLINE);
    if (lines.length < LINE_TO_APPEND - 1) {
      throw new Error("Invalid line length found to apply fix");
    }

    // check if fix is present, otherwise add it and write file
    const lineIndex = LINE_TO_APPEND - 1;
    if (!lines[lineIndex].includes(CODE_TO_APPEND.trim())) {
      lines.splice(lineIndex, 0, [CODE_TO_APPEND]);
      await fs.writeFile(connectionPath, lines.join(NEWLINE), FILE_FORMAT);

      console.log("OK: Writing fix to file...");
    } else {
      console.log("OK: Fix already present, no need to write to file");
    }
  } catch (err) {
    console.log("ERROR:", err);
    process.exit(1);
  }
}
main();
