// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

/** This contains a class which permits easy (ish) setup of callbacks.
 *
 * To use it you have to create a static map of an array of callbacks like this
 * (and sadly it must be a static)
 *
 *
 * typedef Callback_Context_Base<My_Plugin> Callbacks;
 * template <> Callbacks::Contexts Callbacks::contexts = {};
 *
 * Then set up as many contexts as you'll need with
 *
 * contexts[id] = std::make_unique<Callback_Context<My_Plugin, id>>()
 *
 * and populate contexts as necessary with
 *
 * callback_fn = Callbacks::contexts[entry]->reserve(this, callback);
 *
 * Note that reserve will return nullptr if the entry is already in use.
 *
 * You can release an entry with
 *
 * Callbacks::contexts[entry]->free(this, callback);
 */
#include <memory>
#include <unordered_map>

// Stolen from PluginInterface.h
using PFUNCPLUGINCMD = void(__cdecl *)();

template <class Callback_Class>
class Callback_Context_Base
{
    using Callback_Type = void (Callback_Class::*)();
    using Const_Callback_Type = void (Callback_Class::*)() const;

  public:
    explicit Callback_Context_Base(PFUNCPLUGINCMD callback) noexcept :
        callback_(callback),
        instance_(nullptr),
        method_(nullptr)
    {
    }

    /** Allocate callback slot.
     *
     * Returns a callback function, or nullptr if the slot is already reserved.
     */
    PFUNCPLUGINCMD reserve(
        Callback_Class *instance, Callback_Type method
    ) noexcept
    {
        if (instance_ != nullptr)
        {
            return nullptr;
        }

        instance_ = instance;
        method_ = method;    // NOLINT(cppcoreguidelines-pro-type-union-access)
        return callback_;
    }

    PFUNCPLUGINCMD reserve(
        Callback_Class *instance, Const_Callback_Type method
    ) noexcept
    {
        if (instance_ != nullptr)
        {
            return nullptr;
        }

        instance_ = instance;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        const_method_ = method;
        return callback_;
    }

    /** This is for setting up menu separators.
     *
     * It is a bit of a crock. It always returns nullptr, so you can't actually
     * tell if it worked or not. It does however reserve a slot if successful.
     */
    PFUNCPLUGINCMD reserve(Callback_Class *instance, nullptr_t method) noexcept
    {
        if (instance_ != nullptr)
        {
            return nullptr;
        }

        instance_ = instance;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        const_method_ = method;
        return nullptr;
    }

    /** Release callback slot. */
    void free() noexcept
    {
        instance_ = nullptr;
        // not clearing method_: it won't be used, since instance_ is nullptr
        // and so this entry is marked as free
    }

  protected:
    static void StaticInvoke(int context)
    {
#pragma warning(push)
#pragma warning(disable : 26446 26482)
        // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
        return std::invoke(
            contexts[context]->method_, contexts[context]->instance_
        );
        // NOLINTEND(cppcoreguidelines-pro-type-union-access)
#pragma warning(pop)
    }

  private:
    PFUNCPLUGINCMD callback_;
    Callback_Class *instance_;
    union
    {
        Callback_Type method_;
        Const_Callback_Type const_method_;
    };

  public:
    using Contexts =
        std::unordered_map<int, std::unique_ptr<Callback_Context_Base>>;
    static Contexts contexts;
};

template <class Callback_Class, int context>
class Callback_Context : public Callback_Context_Base<Callback_Class>
{
    using Super = Callback_Context_Base<Callback_Class>;

  public:
    Callback_Context() noexcept : Super(&GeneratedStaticFunction)
    {
    }

  private:
    static void GeneratedStaticFunction()
    {
        return Super::StaticInvoke(context);
    }
};
