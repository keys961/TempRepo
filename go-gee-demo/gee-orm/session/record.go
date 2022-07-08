package session

import (
	"errors"
	"gee-orm/clause"
	"reflect"
)

// --- Execution: Insert, Find, Delete, Count, Update ---

func (s *Session) Insert(values ...interface{}) (int64, error) {
	// INSERT INTO table
	// VALUES (v1), (v2), ....
	recordValues := make([]interface{}, 0)
	for _, value := range values {
		s.CallMethod(BeforeInsert, value)
		table := s.Model(value).RefTable()
		s.clause.Set(clause.Insert, table.Name, table.FieldNames)
		recordValues = append(recordValues, table.RecordValues(value))
	}

	s.clause.Set(clause.Values, recordValues...)
	sql, vars := s.clause.Build(clause.Insert, clause.Values)
	result, err := s.Raw(sql, vars...).Exec()
	if err != nil {
		return 0, err
	}
	s.CallMethod(AfterInsert, nil)
	return result.RowsAffected()
}

func (s *Session) Find(values interface{}) error {
	s.CallMethod(BeforeQuery, nil)
	// find all values and fill it into values
	destSlice := reflect.Indirect(reflect.ValueOf(values))
	destType := destSlice.Type().Elem()
	table := s.Model(reflect.New(destType).Elem().Interface()).RefTable()

	s.clause.Set(clause.Select, table.Name, table.FieldNames)
	sql, vars := s.clause.Build(clause.Select, clause.Where, clause.OrderBy, clause.Limit)
	rows, err := s.Raw(sql, vars...).QueryRows()
	if err != nil {
		return err
	}

	for rows.Next() {
		dest := reflect.New(destType).Elem()
		var values []interface{}
		// fetch record fields
		for _, name := range table.FieldNames {
			values = append(values, dest.FieldByName(name).Addr().Interface())
		}
		// fill them
		// no need to set value because they're pointers/references
		if err = rows.Scan(values...); err != nil {
			return err
		}
		s.CallMethod(AfterQuery, dest.Addr().Interface())
		// append
		destSlice.Set(reflect.Append(destSlice, dest))
	}
	return rows.Close()
}

func (s *Session) Update(kv ...interface{}) (int64, error) {
	// UPDATE table
	// SET k1=v1, k2=v2, ...
	s.CallMethod(BeforeUpdate, nil)
	m, ok := kv[0].(map[string]interface{})
	if !ok {
		m = make(map[string]interface{})
		for i := 0; i < len(kv); i += 2 {
			m[kv[i].(string)] = kv[i+1]
		}
	}
	s.clause.Set(clause.Update, s.RefTable().Name, m)
	sql, vars := s.clause.Build(clause.Update, clause.Where)
	result, err := s.Raw(sql, vars...).Exec()
	if err != nil {
		return 0, err
	}
	s.CallMethod(AfterUpdate, nil)
	return result.RowsAffected()
}

func (s *Session) Delete() (int64, error) {
	// DELETE FROM table
	s.CallMethod(BeforeDelete, nil)
	s.clause.Set(clause.Delete, s.RefTable().Name)
	sql, vars := s.clause.Build(clause.Delete, clause.Where)
	result, err := s.Raw(sql, vars...).Exec()
	if err != nil {
		return 0, err
	}
	s.CallMethod(AfterDelete, nil)
	return result.RowsAffected()
}

func (s *Session) Count() (int64, error) {
	// SELECT count(*) FROM table
	s.clause.Set(clause.Count, s.RefTable().Name)
	sql, vars := s.clause.Build(clause.Count, clause.Where)
	row := s.Raw(sql, vars...).QueryRow()
	var cnt int64
	if err := row.Scan(&cnt); err != nil {
		return 0, err
	}
	return cnt, nil
}

func (s *Session) First(value interface{}) error {
	dest := reflect.Indirect(reflect.ValueOf(value))
	destSlice := reflect.New(reflect.SliceOf(dest.Type())).Elem()
	if err := s.Limit(1).Find(destSlice.Addr().Interface()); err != nil {
		return err
	}
	if destSlice.Len() == 0 {
		return errors.New("not found")
	}
	dest.Set(destSlice.Index(0))
	return nil
}

// --- Chain Invoke: Where, Limit, OrderBy, Having, GroupBy ---

func (s *Session) Limit(num int) *Session {
	s.clause.Set(clause.Limit, num)
	return s
}

func (s *Session) Where(desc string, args ...interface{}) *Session {
	var vars []interface{}
	// vars[0] = desc, vars[1:] = vars
	s.clause.Set(clause.Where, append(append(vars, desc), args...)...)
	return s
}

func (s *Session) OrderBy(desc string) *Session {
	s.clause.Set(clause.OrderBy, desc)
	return s
}

func (s *Session) Having(desc string, args ...interface{}) *Session {
	var vars []interface{}
	// vars[0] = desc, vars[1:] = vars
	s.clause.Set(clause.Having, append(append(vars, desc), args...)...)
	return s
}

func (s *Session) GroupBy(desc ...string) *Session {
	s.clause.Set(clause.GroupBy, desc)
	return s
}
