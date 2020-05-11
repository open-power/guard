// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <filesystem>

namespace openpower
{
namespace guard
{
namespace fs = std::filesystem;
/**
 * @class GuardFile
 *
 * Cater for performing read/write operations on the guard file
 */
class GuardFile
{
  public:
    GuardFile() = delete;
    ~GuardFile() = default;
    GuardFile(const GuardFile&) = delete;
    GuardFile& operator=(const GuardFile&) = delete;
    GuardFile(GuardFile&&) = delete;
    GuardFile& operator=(GuardFile&&) = delete;

    /**
     * @brief Constructor
     *
     * Fills in this class's data fields from the stream.
     *
     * @param[in] file GUARD file path
     */
    explicit GuardFile(const fs::path& file);

    /**
     * @brief Check the file exists or not in the pnor partition.
     *
     * @return NULL
     */
    void initialize();

    /**
     * @brief Read guard data from the guard partition.
     *
     * @param[in] pos position in the file to read guard data
     * @param[in] dst data to read
     * @param[in] len length of the data to read
     *            magic number and other details.
     * @return NULL
     */
    void read(const uint64_t pos, void* dst, const uint64_t len);

    /**
     * @brief Write guard data to the guard partition.
     *
     * @param[in] pos position in the file to write guard data
     * @param[in] dst data to write
     * @param[in] len length of the data to write
     * @return NULL
     */
    void write(const uint64_t pos, const void* src, const uint64_t len);

    /**
     * @brief Erase guard data from the guard partition.
     *
     * @param[in] pos position in the file to erase guard data
     * @param[in] len length of the data to erase from position
     * @return NULL
     */
    void erase(const uint64_t pos, const uint64_t len);

    /**
     * @brief Return size of guard file
     *
     * @return size of the guard partition file
     */
    uint32_t size();

  private:
    fs::path guardFile;
    uint32_t fileSize = 0;
};
} // namespace guard
} // namespace openpower
