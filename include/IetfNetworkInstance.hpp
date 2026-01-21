#pragma once

#include "IetfInterfaces.hpp"
#include "IetfRouting.hpp"
#include "YangModel.hpp"
#include <libyang/libyang.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace yang {

  class IetfNetworkInstances : public YangModel {
  public:
    class NetworkInstance {
    public:
      NetworkInstance() = default;
      ~NetworkInstance() = default;

      // non-copyable, movable to allow storing in std::vector via move
      NetworkInstance(const NetworkInstance &) = delete;
      NetworkInstance &operator=(const NetworkInstance &) = delete;
      NetworkInstance(NetworkInstance &&) noexcept = default;
      NetworkInstance &operator=(NetworkInstance &&) noexcept = default;

      const std::string &getName() const noexcept { return name_; }
      void setName(const std::string &n) { name_ = n; }

      bool getEnabled() const noexcept { return enabled_; }
      void setEnabled(bool e) { enabled_ = e; }

      const std::optional<std::string> &getDescription() const noexcept {
        return description_;
      }
      void setDescription(const std::optional<std::string> &d) {
        description_ = d;
      }

      // routing / interfaces submodels parsed from mount points
      const IetfRouting *getRouting() const noexcept { return routing_.get(); }
      void setRouting(std::unique_ptr<IetfRouting> r) {
        routing_ = std::move(r);
      }

      const IetfInterfaces *getInterfaces() const noexcept {
        return interfaces_.get();
      }
      void setInterfaces(std::unique_ptr<IetfInterfaces> i) {
        interfaces_ = std::move(i);
      }

    private:
      std::string name_;
      bool enabled_ = true;
      std::optional<std::string> description_;

      std::unique_ptr<IetfRouting> routing_;
      std::unique_ptr<IetfInterfaces> interfaces_;
    };

    IetfNetworkInstances() = default;
    ~IetfNetworkInstances() override = default;

    const std::vector<NetworkInstance> &getNetworkInstances() const noexcept {
      return instances_;
    }
    std::vector<NetworkInstance> &mutableNetworkInstances() noexcept {
      return instances_;
    }

    // YangModel interface
    struct lyd_node *serialize(const YangContext &ctx) const override;
    static std::unique_ptr<IetfNetworkInstances>
    deserialize(const YangContext &ctx, struct lyd_node *tree);

    // ext data callback called by libyang for schema-mount extension instances.
    // Signature matches libyang's ext-data callback: (const lys_ext_instance*,
    // void*) -> void*
    static void *extDataCallback(const struct lys_ext_instance *ext,
                                 void *user_data);

    // ext-data callback used by libyang schema-mount processing.
    // Signature matches libyang's ly_ext_data_clb.
    static LY_ERR extDataCallback(const struct lysc_ext_instance *ext,
                                  const struct lyd_node *node, void *user_data,
                                  void **ext_data, ly_bool *need_free);

  private:
    std::vector<NetworkInstance> instances_;
  };

} // namespace yang
