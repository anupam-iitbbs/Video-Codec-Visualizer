#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace ivcv::core {

class PipelineContext;

/// A single configurable parameter exposed by a pipeline stage (for example
/// the quantization stage exposes an integer "qp" parameter). Using a
/// variant keeps the parameter system open to future stages without
/// changing this header.
using ParameterValue = std::variant<bool, int, double, std::string>;

/// Named parameters exposed by a stage, keyed by parameter name for lookup
/// from the UI parameter panel.
using ParameterSet = std::unordered_map<std::string, ParameterValue>;

/// Common interface implemented by every stage of the compression pipeline
/// (RGB to YUV conversion, chroma subsampling, DCT, quantization, and so
/// on).
///
/// Design rationale: this Strategy-pattern interface lets PipelineController
/// sequence and step through arbitrarily many stages without knowing their
/// concrete types, and lets the UI bind to any stage uniformly. New stages
/// are added by implementing this interface and registering with
/// StageRegistry; neither the controller nor the UI shell need to change.
/// See docs/ARCHITECTURE.md, section 2, for the full class diagram.
class IPipelineStage {
public:
    virtual ~IPipelineStage() = default;

    /// Human-readable, unique name of this stage (for example "RGB to
    /// YUV"). Used for UI labels, logging, and stage lookup.
    [[nodiscard]] virtual std::string_view name() const noexcept = 0;

    /// Executes this stage's transformation, reading its inputs from and
    /// writing its outputs to the shared pipeline context.
    ///
    /// Complexity is defined by the concrete stage and documented on each
    /// override.
    virtual void process(PipelineContext& context) = 0;

    /// Returns the current value of every user-configurable parameter this
    /// stage exposes (for example quantization QP). An empty set means the
    /// stage has no configurable parameters.
    [[nodiscard]] virtual ParameterSet parameters() const = 0;

    /// Restores the stage to its default parameter values and clears any
    /// internal state accumulated by a previous process() call.
    virtual void reset() = 0;
};

}  // namespace ivcv::core
