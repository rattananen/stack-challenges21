#include <iostream>
#include <cstdint>
#include <array>
#include <format>
#include <functional>
#include <bit>
#include <vector>

constexpr std::array<uint16_t, 81> in_board = {
	1,3,0,0,7,0,0,0,0,
	6,0,0,1,9,5,0,0,0,
	0,0,8,0,0,0,0,6,0,
	2,0,0,0,6,0,0,0,3,
	4,0,0,8,0,3,0,0,1,
	7,0,0,0,0,0,0,0,0,
	0,6,0,0,0,0,2,8,0,
	0,0,0,4,1,9,0,0,0,
	0,0,0,0,0,0,0,0,0,
};

constexpr uint16_t min_sum = 19;

constexpr std::array<uint16_t, 32> in_path = {
	0, 0, 1, 0, 2, 0, 2, 1,
	0, 8, 0, 7, 0, 6, 1, 6,
	8, 0, 8, 1, 8, 2, 7, 2,
	8, 8, 7, 8, 6, 8, 6, 7
};

//store in row-wise
struct Grid {
	using value_type = uint16_t;

	value_type& operator[](size_t n);
	const value_type& operator[](size_t n) const;

	value_type& operator()(size_t row, size_t col);
	const value_type& operator()(size_t row, size_t col) const;

	value_type& as_sub(size_t sub_n, size_t n);

	bool is_full();

	bool is_bad();

	std::array<value_type, 81> data;
};


bool is_ok(const Grid& in);
bool is_ok(const Grid& in, size_t row, size_t col);
bool is_path_ok(const Grid& in);

void format_to(std::string& to, const Grid& in);
void format_to_v2(std::string& to, const Grid& in);

struct Candidate_reduce {
	/// @brief use bit represent for candidate instead of integer
	struct Candidate {
		using value_type = Grid::value_type;
		Candidate();
		Candidate(value_type v);

		operator value_type() const;

		size_t size() const;
		void set_all();
		void set(size_t n);
		void clear(size_t n);
		void del(value_type n);
		void push(value_type n);
		void unset(Candidate c);
		value_type pop();
		value_type peek(size_t pos) const;
		value_type data;
	};
	struct Candidate_table {
		Candidate& operator[](size_t n);

		const Candidate& operator[](size_t n) const;

		Candidate& operator()(size_t row, size_t col);

		const Candidate& operator()(size_t row, size_t col) const;

		size_t candidate_size() const;
		bool has_size_1();
		std::array<Candidate, 81> data;
	};



	using locator_type = std::function<size_t(size_t, size_t)>;


	/// @return 0 = error, if error mean there are multiple solution for this grid
	int operator()(Grid& result);

	void refine_as(Candidate_table& ctb, const Grid& in, locator_type fn);
	void refine_full(Candidate_table& ctb, const Grid& in);
	void clear(Candidate_table& ctb, const  Grid& in);
	void do_solve(Candidate_table& ctb, Grid& in);

};

std::ostream& operator<<(std::ostream& os, Candidate_reduce::Candidate m);

std::ostream& operator<<(std::ostream& os, const Candidate_reduce::Candidate_table& tb);

struct Backtrack {
	Backtrack(std::vector<Grid>& all_result);
	int operator()(Grid& result);

	
	bool do_solve(Grid& result);
	bool is_ok(const Grid& in, size_t row, size_t col, Grid::value_type num);

	std::vector<Grid>& m_all_result;
};



int main() {
	Grid result{};
	result.data = in_board;

	std::string print_buf;

	format_to(print_buf, result);

	std::cout << print_buf << "\n";


	Candidate_reduce resol{};

	int err = resol(result);

	if (!err) {
		print_buf.clear();
		format_to(print_buf, result);
		std::cerr << "only 1 solution possible\n";
		std::cout << print_buf << "\n";
		return 0;
	}

	std::vector<Grid> all_result;
	Backtrack bt{ all_result };
	bt(result);

	std::cout << std::format("solution with no filter={}\n\n", all_result.size());

	size_t solution_n = 0;
	print_buf.clear();
	for (auto& it: all_result) {
		if (is_ok(it)) {
			format_to_v2(print_buf, it);
			print_buf += "\n";
			solution_n++;
		}
	}
	std::cout << std::format("solution with filter={}\n\n", solution_n);
	std::cout << print_buf << "\n";

	return 0;
}


void format_to(std::string& to, const Grid& in) {
	for (size_t i = 0; i < 9; ++i) {
		std::format_to(std::back_inserter(to), "{} {} {} {} {} {} {} {} {}\n", in[i * 9], in[i * 9 + 1], in[i * 9 + 2], in[i * 9 + 3], in[i * 9 + 4], in[i * 9 + 5], in[i * 9 + 6], in[i * 9 + 7], in[i * 9 + 8]);
	}
}
void format_to_v2(std::string& to, const Grid& in) {
	for (size_t i = 0; i < 9; ++i) {
		std::format_to(std::back_inserter(to), "{}{}{}{}{}{}{}{}{}\n", in[i * 9], in[i * 9 + 1], in[i * 9 + 2], in[i * 9 + 3], in[i * 9 + 4], in[i * 9 + 5], in[i * 9 + 6], in[i * 9 + 7], in[i * 9 + 8]);
	}
}

Grid::value_type& Grid::operator[](size_t n)
{
	return data[n];
}

const Grid::value_type& Grid::operator[](size_t n) const
{
	return data[n];
}

const Grid::value_type& Grid::operator()(size_t row, size_t col) const
{
	return data[row * 9 + col];
}

Grid::value_type& Grid::as_sub(size_t sub_n, size_t n)
{
	return data[(27 * (sub_n / 3) + 3 * (sub_n % 3)) + (9 * (n / 3) + (n % 3))];
}

Grid::value_type& Grid::operator()(size_t row, size_t col)
{
	return data[row * 9 + col];
}

bool Grid::is_full()
{
	for (auto v : data) {
		if (v == 0) {
			return false;
		}
	}
	return true;
}

bool Grid::is_bad()
{
	for (auto v : data) {
		if (v > 9 || v < 0) {
			return true;
		}
	}
	return false;
}

bool is_ok(const Grid& in)
{
	for (size_t row = 0; row < 9; ++row) {
		for (size_t col = 0; col < 9; ++col) {
			if (in(row, col) == 0) {
				continue;
			}
			if (!is_ok(in, row, col)) {
				return false;
			}
		}
	}
	return is_path_ok(in);
}

bool is_ok(const Grid& in, size_t row, size_t col) {
	auto num = in(row, col);
	for (size_t n = 0; n < 9; ++n) {
		if (n != col && in(row, n) == num) {
			return false;
		}
	}

	for (size_t n = 0; n < 9; ++n) {
		if (n != row && in(n, col) == num) {
			return false;
		}
	}

	size_t start_row = row - row % 3, start_col = col - col % 3;
	for (size_t n = 0; n < 3; ++n) {
		for (size_t k = 0; k < 3; ++k) {
			auto x = n + start_row;
			auto y = k + start_col;
			if (!(x == row && y == col) && in(x, y) == num)
				return false;
		}
	}
	return true;
}


bool is_path_ok(const Grid& in)
{
	for (size_t m = 0; m < 4; ++m) {
		uint16_t sum = 0;
		for (size_t n = 0; n < 4; ++n) {
			auto row = in_path[m * 4 + n * 2];
			auto col = in_path[m * 4 + n * 2 + 1];
			auto v = in(row, col);
			if (v == 0) {
				sum = min_sum; //make valid and go next path
				break;
			}
			sum += v;
		}
		if (sum < min_sum) {
			return false;
		}
	}
	return true;
}

Candidate_reduce::Candidate::Candidate() :data{ 0b1'1111'1111 }
{
}

Candidate_reduce::Candidate::Candidate(Candidate_reduce::Candidate::value_type v) :data{ v }
{
}

Candidate_reduce::Candidate::operator typename Candidate_reduce::Candidate::value_type() const
{
	return data;
}

size_t Candidate_reduce::Candidate::size() const
{
	return std::popcount(data);
}

void Candidate_reduce::Candidate::set_all()
{
	data = 0b1'1111'1111;
}

void Candidate_reduce::Candidate::set(size_t n)
{
	data |= 1 << n;
}

void Candidate_reduce::Candidate::clear(size_t n)
{
	data &= ~(1 << n);
}

void Candidate_reduce::Candidate::del(Candidate_reduce::Candidate::value_type n)
{
	data &= ~(1 << (n - 1));
}

void Candidate_reduce::Candidate::push(Candidate_reduce::Candidate::value_type n)
{
	data |= 1 << (n - 1);
}

void Candidate_reduce::Candidate::unset(Candidate_reduce::Candidate c)
{
	data &= ~c.data;
}

Candidate_reduce::Candidate::value_type Candidate_reduce::Candidate::pop()
{
	for (size_t i = 0, cur = 0; i < 9; ++i) {
		value_type mask = 1 << i;
		if ((data & mask) == mask) {
			clear(i);
			return i + 1;
		}
	}
	return 0;
}

Candidate_reduce::Candidate::value_type Candidate_reduce::Candidate::peek(size_t pos) const
{
	for (size_t i = 0, cur = 0; i < 9; ++i) {
		value_type mask = 1 << i;
		if ((data & mask) == mask) {
			if (cur == pos) {
				return i + 1;
			}
			else {
				++cur;
			}
		}
	}
	return 0;
}

Candidate_reduce::Candidate& Candidate_reduce::Candidate_table::operator[](size_t n)
{
	return data[n];
}

const Candidate_reduce::Candidate& Candidate_reduce::Candidate_table::operator[](size_t n) const
{
	return data[n];
}

Candidate_reduce::Candidate& Candidate_reduce::Candidate_table::operator()(size_t row, size_t col)
{
	return data[row * 9 + col];
}

const Candidate_reduce::Candidate& Candidate_reduce::Candidate_table::operator()(size_t row, size_t col) const
{
	return data[row * 9 + col];
}

size_t Candidate_reduce::Candidate_table::candidate_size() const
{
	size_t n{};
	for (auto v : data) {
		n += v.size();
	}
	return n;
}

bool Candidate_reduce::Candidate_table::has_size_1()
{
	for (auto v : data) {
		if (v.size() == 1) {
			return true;
		}
	}
	return false;
}

int Candidate_reduce::operator()(Grid& result)
{
	Candidate_table ctb{};

	refine_full(ctb, result);


	while (ctb.has_size_1()) {
		do_solve(ctb, result);
		refine_full(ctb, result);
	}


	if (ctb.candidate_size() == 0) {
		return  0;
	}

	return 1;
}

void Candidate_reduce::refine_as(Candidate_reduce::Candidate_table& out, const Grid& in, locator_type fn)
{
	for (size_t i = 0; i < 9; ++i) {
		Candidate cd{ 0 };
		for (size_t j = 0; j < 9; ++j) {
			size_t n = fn(i, j);
			if (in[n] != 0) {
				cd.push(in[n]);
			}
		}
		for (size_t j = 0; j < 9; ++j) {
			out[fn(i, j)].unset(cd);
		}
	}
}

void Candidate_reduce::refine_full(Candidate_reduce::Candidate_table& ctb, const Grid& in)
{
	clear(ctb, in);
	refine_as(ctb, in, [](size_t row, size_t col) { return row * 9 + col; });
	refine_as(ctb, in, [](size_t col, size_t row) { return row * 9 + col; });
	refine_as(ctb, in, [](size_t sub_n, size_t n) { return (27 * (sub_n / 3) + 3 * (sub_n % 3)) + (9 * (n / 3) + (n % 3)); });
}

void Candidate_reduce::clear(Candidate_reduce::Candidate_table& ctb, const Grid& in)
{
	for (size_t i = 0; i < 81; ++i) {
		if (in[i] != 0) {
			ctb[i] = 0;
		}
	}
}

void Candidate_reduce::do_solve(Candidate_reduce::Candidate_table& ctb, Grid& in)
{
	for (size_t i = 0; i < 81; ++i) {
		if (ctb[i].size() == 1) {
			in[i] = ctb[i].peek(0);
		}
	}
}

std::ostream& operator<<(std::ostream& os, Candidate_reduce::Candidate m)
{
	for (size_t i = 0; i < 9; ++i) {
		Candidate_reduce::Candidate::value_type mask = 1 << i;
		if ((m.data & mask) == mask) {
			os << (i + 1);
		}
		else {
			os << "_";
		}
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const Candidate_reduce::Candidate_table& tb)
{
	for (size_t i = 0; i < 81; ++i) {
		auto pos = i + 1;
		os << tb[i] << ' ';
		if (pos % 3 == 0) {
			os << "  ";
		}
		if (pos % 9 == 0) {
			os << '\n';
		}
		if (pos % 27 == 0) {
			os << '\n';
		}
	}
	return os;
}

Backtrack::Backtrack(std::vector<Grid>& all_result) :m_all_result{ all_result }
{
}

int Backtrack::operator()(Grid& result) {
	if (do_solve(result)) {
		return 0;
	}
	return 1;
}

bool Backtrack::do_solve(Grid& result)
{
	size_t row, col;

	for (row = 0; row < 9; ++row) {
		for (col = 0; col < 9; ++col) {
			if (result(row, col) == 0) {
				goto solve;
			}
		}
	}
	m_all_result.push_back(result);
	return false; //nothing to solve
solve:

	for (Grid::value_type num = 1; num <= 9; ++num) {
		if (is_ok(result, row, col, num)) {
			result(row, col) = num;
			if (do_solve(result)) {
				return true;
			}

			result(row, col) = 0; // Backtrack
		}
	}
	return false; //trigger Backtrack
}





bool Backtrack::is_ok(const Grid& in, size_t row, size_t col, Grid::value_type num)
{
	for (size_t n = 0; n < 9; ++n) {
		if (in(row, n) == num) {
			return false;
		}
	}

	for (size_t n = 0; n < 9; ++n) {
		if (in(n, col) == num) {
			return false;
		}
	}

	size_t start_row = row - row % 3, start_col = col - col % 3;
	for (size_t n = 0; n < 3; ++n)
		for (size_t k = 0; k < 3; ++k)
			if (in(n + start_row, k + start_col) == num)
				return false;
	return true;
}
