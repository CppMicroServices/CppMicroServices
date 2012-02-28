/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#ifndef USMODULEVERSION_H
#define USMODULEVERSION_H

#include <usExportMacros.h>

#include <string>

US_BEGIN_NAMESPACE

/**
 * \ingroup MicroServices
 *
 * Version identifier for US modules.
 *
 * <p>
 * Version identifiers have four components.
 * <ol>
 * <li>Major version. A non-negative integer.</li>
 * <li>Minor version. A non-negative integer.</li>
 * <li>Micro version. A non-negative integer.</li>
 * <li>Qualifier. A text string. See <code>ModuleVersion(const std::string&)</code> for the
 * format of the qualifier string.</li>
 * </ol>
 *
 * <p>
 * <code>ModuleVersion</code> objects are immutable.
 */
class US_EXPORT ModuleVersion {

private:

  friend class ModulePrivate;

  unsigned int majorVersion;
  unsigned int minorVersion;
  unsigned int microVersion;
  std::string      qualifier;

  static const char SEPARATOR; //  = "."

  bool undefined;


  /**
   * Called by the ModuleVersion constructors to validate the version components.
   *
   * @return <code>true</code> if the validation was successfull, <code>false</code> otherwise.
   */
  void Validate();

  ModuleVersion& operator=(const ModuleVersion& v);

  explicit ModuleVersion(bool undefined = false);

public:

  /**
   * The empty version "0.0.0".
   */
  static ModuleVersion EmptyVersion();

  /**
   * Creates an undefined version identifier, representing either
   * infinity or minus infinity.
   */
  static ModuleVersion UndefinedVersion();

  /**
   * Creates a version identifier from the specified numerical components.
   *
   * <p>
   * The qualifier is set to the empty string.
   *
   * @param majorVersion Major component of the version identifier.
   * @param minorVersion Minor component of the version identifier.
   * @param microVersion Micro component of the version identifier.
   *
   */
  ModuleVersion(unsigned int majorVersion, unsigned int minorVersion, unsigned int microVersion);

  /**
   * Creates a version identifier from the specified components.
   *
   * @param majorVersion Major component of the version identifier.
   * @param minorVersion Minor component of the version identifier.
   * @param microVersion Micro component of the version identifier.
   * @param qualifier Qualifier component of the version identifier.
   */
  ModuleVersion(unsigned int majorVersion, unsigned int minorVersion, unsigned int microVersion, const std::string& qualifier);

  /**
   * Created a version identifier from the specified string.
   *
   * <p>
   * Here is the grammar for version strings.
   *
   * <pre>
   * version ::= majorVersion('.'minorVersion('.'microVersion('.'qualifier)?)?)?
   * majorVersion ::= digit+
   * minorVersion ::= digit+
   * microVersion ::= digit+
   * qualifier ::= (alpha|digit|'_'|'-')+
   * digit ::= [0..9]
   * alpha ::= [a..zA..Z]
   * </pre>
   *
   * There must be no whitespace in version.
   *
   * @param version string representation of the version identifier.
   */
  ModuleVersion(const std::string& version);

  /**
   * Create a version identifier from another.
   *
   * @param version Another version identifier
   */
  ModuleVersion(const ModuleVersion& version);


  /**
   * Parses a version identifier from the specified string.
   *
   * <p>
   * See <code>ModuleVersion(const std::string&)</code> for the format of the version string.
   *
   * @param version string representation of the version identifier. Leading
   *        and trailing whitespace will be ignored.
   * @return A <code>ModuleVersion</code> object representing the version
   *         identifier. If <code>version</code> is the empty string
   *         then <code>EmptyVersion</code> will be
   *         returned.
   */
  static ModuleVersion ParseVersion(const std::string& version);

  /**
   * Returns the undefined state of this version identifier.
   *
   * @return <code>true</code> if this version identifier is undefined,
   *         <code>false</code> otherwise.
   */
  bool IsUndefined() const;

  /**
   * Returns the majorVersion component of this version identifier.
   *
   * @return The majorVersion component.
   */
  unsigned int GetMajor() const;

  /**
   * Returns the minorVersion component of this version identifier.
   *
   * @return The minorVersion component.
   */
  unsigned int GetMinor() const;

  /**
   * Returns the microVersion component of this version identifier.
   *
   * @return The microVersion component.
   */
  unsigned int GetMicro() const;

  /**
   * Returns the qualifier component of this version identifier.
   *
   * @return The qualifier component.
   */
  std::string GetQualifier() const;

  /**
   * Returns the string representation of this version identifier.
   *
   * <p>
   * The format of the version string will be <code>majorVersion.minorVersion.microVersion</code>
   * if qualifier is the empty string or
   * <code>majorVersion.minorVersion.microVersion.qualifier</code> otherwise.
   *
   * @return The string representation of this version identifier.
   */
  std::string ToString() const;

  /**
   * Compares this <code>ModuleVersion</code> object to another object.
   *
   * <p>
   * A version is considered to be <b>equal to </b> another version if the
   * majorVersion, minorVersion and microVersion components are equal and the qualifier component
   * is equal.
   *
   * @param object The <code>ModuleVersion</code> object to be compared.
   * @return <code>true</code> if <code>object</code> is a
   *         <code>ModuleVersion</code> and is equal to this object;
   *         <code>false</code> otherwise.
   */
  bool operator==(const ModuleVersion& object) const;

  /**
   * Compares this <code>ModuleVersion</code> object to another object.
   *
   * <p>
   * A version is considered to be <b>less than </b> another version if its
   * majorVersion component is less than the other version's majorVersion component, or the
   * majorVersion components are equal and its minorVersion component is less than the other
   * version's minorVersion component, or the majorVersion and minorVersion components are equal
   * and its microVersion component is less than the other version's microVersion component,
   * or the majorVersion, minorVersion and microVersion components are equal and it's qualifier
   * component is less than the other version's qualifier component (using
   * <code>std::string::operator<()</code>).
   *
   * <p>
   * A version is considered to be <b>equal to</b> another version if the
   * majorVersion, minorVersion and microVersion components are equal and the qualifier component
   * is equal.
   *
   * @param object The <code>ModuleVersion</code> object to be compared.
   * @return A negative integer, zero, or a positive integer if this object is
   *         less than, equal to, or greater than the specified
   *         <code>ModuleVersion</code> object.
   */
  int Compare(const ModuleVersion& object) const;

};

US_END_NAMESPACE

/**
 * \ingroup MicroServices
 */
US_EXPORT std::ostream& operator<<(std::ostream& os, const US_PREPEND_NAMESPACE(ModuleVersion)& v);

#endif // USMODULEVERSION_H
