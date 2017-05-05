#ifndef ANGELSCRIPTINTEGRATION_CHANGEWORKINGDIR_H
#define ANGELSCRIPTINTEGRATION_CHANGEWORKINGDIR_H

#include <QDir>

namespace AngelScriptIntegration {

class ChangeWorkingDir final
{
public:
  ChangeWorkingDir(const QDir& currentDir);
  ~ChangeWorkingDir();

  ChangeWorkingDir(const ChangeWorkingDir&) = delete;
  ChangeWorkingDir(ChangeWorkingDir&&) = delete;
  ChangeWorkingDir& operator=(const ChangeWorkingDir&) = delete;
  ChangeWorkingDir& operator=(ChangeWorkingDir&&) = delete;

private:
  const QDir oldDir;
};

} // namespace AngelScriptIntegration

#endif // ANGELSCRIPTINTEGRATION_CHANGEWORKINGDIR_H
