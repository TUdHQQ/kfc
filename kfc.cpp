#include "kfc.h"

int main() {
  let c = readConfig();
  auto data = readData(fs::current_path() / c["config"].asString());

  std::vector<base> bases; // TODO
  std::vector<diff> diffs;
  std::map<std::string, fgname> fgnames;
  std::vector<fgalias> fgaliases;

  readSinfo(fs::current_path() / c["sinfo"].asString(), data, bases, diffs,
            fgnames, fgaliases);

  for (auto &fg : fgaliases) {
    for (auto &diff : diffs) {
      let outpath = fs::current_path() / "output" /
                    (diff.info.name + "_" + fg.name + ".png");
      work(outpath, c["base"].asString(), diff, fg);
    }
  }

  return 0;
}