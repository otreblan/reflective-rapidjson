## Reflection-related: requires extending generator
- [x] Test with (multiple) inheritance
- [x] Test multiple input files
- [x] Add appropriate error handling for de-serialization
- [x] Add reflector based on Boost.Hana
- [ ] Add another generator to prove expandability: maybe for getting members by name in general, similar to one of the proposals
- [x] Add documentation (install instructions, usage)
- [ ] Support enums (undoable with Boost.Hana)
- [ ] Support templated classes

## Library-only
- [ ] Support `std::unique_ptr` and `std::shared_ptr`
- [ ] Support `std::map` and `std::unordered_map`
- [ ] Support `std::any`
- [ ] Support/document customized (de)serialization (eg. serialize some `DateTime` object to ISO string representation)